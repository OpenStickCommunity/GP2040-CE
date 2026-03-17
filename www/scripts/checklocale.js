import { execSync } from 'child_process';
import { spawn } from 'child_process';
import { parseArgs } from 'node:util';

function printUsage() {
	console.log(
		'usage:      npm run check-locale -- [option]\n' +
			'on Windows: node ./scripts/checklocale.js [option]\n' +
			'options:\n' +
			'  -l|--locale <locale> Check changes in baselocale from last change in <locale>\n' +
			'    de de-DE  Check changes in en from last change in de-DE\n' +
			'    fr fr-FR  Check changes in en from last change in fr-FR\n' +
			'    ja ja-JP  Check changes in en from last change in ja-JP\n' +
			'    ko ko-KR  Check changes in en from last change in ko-KR\n' +
			'    pt pt-BR  Check changes in en from last change in pt-BR\n' +
			'    tr tr-TR  Check changes in en from last change in tr-TR\n' +
			'    zh zh-CN  Check changes in en from last change in zh-CN\n' +
			'  -f|--from <history> go back <history> commits in <locale> (default: 1)\n' +
			'  -t|--to <history> which commit to diff against (default: 0 = HEAD)\n' +
			'  -b|--baselocale <locale> Locale folder to get the diffs in (default: en)\n' +
			'    en        Get diffs in en Locale directory (default)\n' +
			'    de de-DE  Get diffs in de-DE Locale directory\n' +
			'    fr fr-FR  Get diffs in fr-FR Locale directory\n' +
			'    ja ja-JP  Get diffs in ja-JP Locale directory\n' +
			'    ko ko-KR  Get diffs in ko-KR Locale directory\n' +
			'    pt pt-BR  Get diffs in pt-BR Locale directory\n' +
			'    tr tr-TR  Get diffs in tr-TR Locale directory\n' +
			'    zh zh-CN  Get diffs in zh-CN Locale directory\n' +
			'Example 1: check changes in en locale from last change in de-DE\n' +
			'  npm run check-locale -- -l de\n' +
			'Example 2: check changes in en locale from 3rd previous commit to 2nd previous commit in zh-CN\n' +
			'  npm run check-locale -- -l zh -f 3 -t 2\n' +
			'Example 2: check changes in ko-KR locale from 2nd previous commit to HEAD in ja-JP\n' +
			'  npm run check-locale -- -l ja -b ko -f 2',
	);
}

function getLocaleDir(locale) {
	switch (locale) {
		case 'de':
		case 'de-DE':
			return './src/Locales/de-DE/';
		case 'en':
			return './src/Locales/en/';
		case 'fr':
		case 'fr-FR':
			return './src/Locales/fr-FR/';
		case 'ja':
		case 'ja-JP':
			return './src/Locales/ja-JP/';
		case 'ko':
		case 'ko-KR':
			return './src/Locales/ko-KR/';
		case 'pt':
		case 'pt-BR':
			return './src/Locales/pt-BR/';
		case 'tr':
		case 'tr-TR':
			return './src/Locales/tr-TR/';
		case 'zh':
		case 'zh-CN':
			return './src/Locales/zh-CN/';
		default:
			console.log(`${locale} is not a valid locale`);
			printUsage();
			process.exit(1);
	}
}

function getHashArry(targetDir) {
	let hashArry;
	try {
		// get diff hashes (max 100 to avoid tracking unnecessarily long history)
		hashArry = execSync(`git log -n 100 --pretty=format:%H -- ${targetDir}`)
			.toString()
			.split('\n')
			.map((line) => line.trim())
			.filter((line) => line !== '');
		if (hashArry.length == 0) {
			console.error(`No commits found for the directory: ${targetDir}`);
			process.exit(1);
		}
		hashArry.unshift('HEAD');
	} catch (error) {
		console.error(`Error executing git commands: ${error.message}`);
		process.exit(1);
	}
	return hashArry;
}

function getDiff(fromHash, toHash, diffPath) {
	console.log(
		`Running git diff from commit ${fromHash} to ${toHash} for ${diffPath}...`,
	);
	let fromLog = execSync(`git log -1 --format="%an <%ae> %ad %n%B" ${fromHash}`)
		.toString()
		.trim();
	let toLog = execSync(`git log -1 --format="%an <%ae> %ad %n%B" ${toHash}`)
		.toString()
		.trim();

	console.log(
		'------------------------------------------------------------------------\n' +
			`commit log of ${fromHash}:\n` +
			`${fromLog}\n` +
			'------------------------------------------------------------------------\n' +
			`commit log of ${toHash}:\n` +
			`${toLog}`,
	);

	const gitDiff = spawn('git', ['diff', fromHash, toHash, '--', diffPath], {
		stdio: 'inherit',
	});

	gitDiff.on('error', (err) => {
		console.error(`Error executing git diff: ${err.message}`);
		process.exit(1);
	});

	gitDiff.on('close', (code) => {
		if (code !== 0) {
			console.error(`git diff process exited with code ${code}`);
			process.exit(code);
		}
	});
}

function checkLocale() {
	const options = {
		help: {
			type: 'boolean',
			short: 'h',
		},
		locale: {
			type: 'string',
			short: 'l',
		},
		baselocale: {
			type: 'string',
			short: 'b',
			default: 'en',
		},
		from: {
			type: 'string',
			short: 'f',
			default: '1',
		},
		to: {
			type: 'string',
			short: 't',
			default: '0',
		},
	};
	let args;
	try {
		args = parseArgs({ options });
	} catch (error) {
		// unexpected option passed (positional) or user is on Windows and options are not properly handled through npm.
		console.log(
			'Error parsing arguments.  Try using `node ./scripts/checklocale.js` on Windows instead.',
		);
		printUsage();
		process.exit(1);
	}

	let targetDir;
	let hashArry;
	let diffFrom;
	let diffTo;
	let diffPath;

	if (args.values.help) {
		printUsage();
		process.exit(0);
	}

	if (!args.values.locale) {
		console.log('Error: The --locale option is required');
		printUsage();
		process.exit(1);
	}

	// set the target diff history id
	diffFrom = parseInt(args.values.from, 10);
	diffTo = parseInt(args.values.to, 10);

	// get target directory for the locale
	targetDir = getLocaleDir(args.values.locale);

	diffPath = getLocaleDir(args.values.baselocale);

	// get list of commit hashes for targetDir
	hashArry = getHashArry(targetDir);

	getDiff(hashArry[diffFrom], hashArry[diffTo], diffPath);
}

checkLocale();
