import { execSync } from "child_process";
import { spawn } from "child_process";
import { Hash } from "crypto";
import { parseArgs } from "node:util";

function printUsage() {
	console.log("usage: npm run check-locale -- [option]");
	console.log("options:");
	console.log(
		"  -l|--locale <locale> Check changes in en from last change in <locale>",
	);
	console.log("    de de-DE  Check changes in en from last change in de-JP");
	console.log("    ja ja-JP  Check changes in en from last change in ja-JP");
	console.log("    ko ko-KR  Check changes in en from last change in ko-KR");
	console.log("    pt pt-BR  Check changes in en from last change in pt-BR");
	console.log("    zh zh-CN  Check changes in en from last change in zh-CN");
	console.log("  -f|--from <history> how many commits to go back");
	console.log("  -t|--to <history> which commit to diff against (0 = HEAD)");
}

const options = {
	"locale": {
		type: "string",
		short: "l",
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

if (!args.values.locale) {
	console.log("Error: The --locale option is required");
	printUsage();
	process.exit(1);
}

//TODO: parse --from annd --to
console.log(`from : ${args.values.from}`);
console.log(`to   : ${args.values.to}`);

let targetDir;

switch (args.values.locale) {
	case "de":
	case "de-DE":
		targetDir = "./src/Locales/de-DE/";
		break;
	case "ja":
	case "ja-JP":
		targetDir = "./src/Locales/ja-JP/";
		break;
	case "ko":
	case "ko-KR":
		targetDir = "./src/Locales/ko-KR/";
		break;
	case "pt":
	case "pt-BR":
		targetDir = "./src/Locales/pt-BR/";
		break;
	case "zh":
	case "zh-CN":
		targetDir = "./src/Locales/zh-CN/";
		break;
	default:
		console.log(`${args.values.locale} is not a valid locale`);
		printUsage();
		process.exit(1);
}

const diffPath = "./src/Locales/en/";

let latestCommitHash;
let CommitHashes; // for development

try {
	// Find the latest commit hash that modified the target locale directory
	latestCommitHash = execSync(`git log -n 1 --pretty=format:%H -- ${targetDir}`)
		.toString().trim();

	CommitHashes = execSync(`git log -n 100 --pretty=format:%H -- ${targetDir}`)
		.toString().trim();
	let HashArry = CommitHashes.split("\n").map(line => line.trim()).filter(line => line !== "");
	console.log(HashArry);

	if (!latestCommitHash) {
		console.error(`No commits found for the directory: ${targetDir}`);
		process.exit(1);
	}
} catch (error) {
	console.error(`Error executing git commands: ${error.message}`);
	process.exit(1);
}

console.log(
	`Running git diff from commit ${latestCommitHash} to HEAD for ${diffPath}...`,
);
const gitDiff = spawn(
	"git",
	["diff", latestCommitHash, "HEAD", "--", diffPath],
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
