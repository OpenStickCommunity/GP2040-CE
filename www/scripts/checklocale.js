import { execSync } from "child_process";
import { spawn } from "child_process";
import { parseArgs } from "node:util";

function printUsage() {
	console.log("usage: npm run check-locale -- [option]");
	console.log("options:");
	console.log(
		"  -l|--locale <locale> Check changes in baselocale from last change in <locale>",
	);
	console.log("    de de-DE  Check changes in en from last change in de-JP");
	console.log("    ja ja-JP  Check changes in en from last change in ja-JP");
	console.log("    ko ko-KR  Check changes in en from last change in ko-KR");
	console.log("    pt pt-BR  Check changes in en from last change in pt-BR");
	console.log("    zh zh-CN  Check changes in en from last change in zh-CN");
	console.log(
		"  -b|--baselocale <locale> Locale folder to get the diffs in (default: en)",
	);
	console.log("    en        Get diffs in en Locale directory (default)");
	console.log("    de de-DE  Get diffs in de-JP Locale directory");
	console.log("    ja ja-JP  Get diffs in ja-JP Locale directory");
	console.log("    ko ko-KR  Get diffs in ko-KR Locale directory");
	console.log("    pt pt-BR  Get diffs in pt-BR Locale directory");
	console.log("    zh zh-CN  Get diffs in zh-CN Locale directory");
	console.log("  -f|--from <history> how many commits to go back");
	console.log("  -t|--to <history> which commit to diff against (0 = HEAD)");
}

function getLocaleDir(locale) {
	switch (locale) {
		case "de":
		case "de-DE":
			return "./src/Locales/de-DE/";
		case "en":
			return "./src/Locales/en/";
		case "ja":
		case "ja-JP":
			return "./src/Locales/ja-JP/";
		case "ko":
		case "ko-KR":
			return "./src/Locales/ko-KR/";
		case "pt":
		case "pt-BR":
			return "./src/Locales/pt-BR/";
		case "zh":
		case "zh-CN":
			return "./src/Locales/zh-CN/";
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
		let CommitHashes = execSync(`git log -n 100 --pretty=format:%H -- ${targetDir}`)
			.toString().trim();
		hashArry = CommitHashes.split("\n").map(line => line.trim()).filter(line => line !== "");
		hashArry.unshift("HEAD");

		if (!hashArry) {
			console.error(`No commits found for the directory: ${targetDir}`);
			process.exit(1);
		}
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
	let fromLog = execSync(`git log -1 --format="%an <%ae> %ad %n%B" ${fromHash}`).toString().trim();
	let toLog = execSync(`git log -1 --format="%an <%ae> %ad %n%B" ${toHash}`).toString().trim();

	console.log(`commit log of ${fromHash}:`)
	console.log(fromLog);
	console.log(`commit log of ${toHash}:`)
	console.log(toLog);

	const gitDiff = spawn(
		"git",
		["diff", fromHash, toHash, "--", diffPath],
		{ stdio: "inherit" },
	);

	gitDiff.on("error", (err) => {
		console.error(`Error executing git diff: ${err.message}`);
		process.exit(1);
	});

	gitDiff.on("close", (code) => {
		if (code !== 0) {
			console.error(`git diff process exited with code ${code}`);
			process.exit(code);
		}
	});
}

function checkLocale() {
	const options = {
		"help": {
			type: "boolean",
			short: "h"
		},
		"locale": {
			type: "string",
			short: "l",
		},
		"baselocale": {
			type: "string",
			short: "b",
			default: "en"
		},
		"from": {
			type: "string",
			short: "f",
			default: "1",
		},
		"to": {
			type: "string",
			short: "t",
			default: "0",
		},
	};
	const args = parseArgs({ options });

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
		console.log("Error: The --locale option is required");
		printUsage();
		process.exit(1);
	}

	// set the target diff history id
	diffFrom = parseInt(args.values.from, 10);
	diffTo = parseInt(args.values.to, 10);

	// get target directory for the locale
	targetDir = getLocaleDir(args.values.locale)

	diffPath = getLocaleDir(args.values.baselocale)

	// get list of commit hashes for targetDir
	hashArry = getHashArry(targetDir);

	getDiff(hashArry[diffFrom], hashArry[diffTo], diffPath);
}

checkLocale();
