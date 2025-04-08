import {execSync} from "child_process";
import { spawn } from "child_process";

function printUsage() {
    console.log("usage: npm run check-locale -- [option]");
    console.log("options:");
    console.log("  -d|--de  Check changes in en from last change in de-DE");
    console.log("  -j|--ja  Check changes in en from last change in ja-JP");
    console.log("  -k|--ko  Check changes in en from last change in ko-KR");
    console.log("  -p|--pt  Check changes in en from last change in pt-BR");
    console.log("  -z|--zh  Check changes in en from last change in zh-CN");
    console.log("  Only one option is allowed.");
}

if (process.argv.length < 3) {
    printUsage();
    process.exit(1);
}

let targetDir;
const args = process.argv.slice(2);
switch (args[0]) {
    case "-d":
    case "--de":
        targetDir = "./src/Locales/de-DE/";
        break;
    case "-j":
    case "--ja":
        targetDir = "./src/Locales/ja-JP/";
        break;
    case "-k":
    case "--ko":
        targetDir = "./src/Locales/ko-KR/";
        break;
    case "-p":
    case "--pt":
        targetDir = "./src/Locales/pt-BR/";
        break;
    case "-z":
    case "--zh":
        targetDir = "./src/Locales/zh-CN/";
        break;
    default:
        console.log(`Unknown option ${args[0]}`);
        printUsage();
        process.exit(1);
}

const diffPath = "./src/Locales/en/";

let latestCommitHash;

try {
    // Find the latest commit hash that modified the target locale directory
    latestCommitHash = execSync(`git log -n 1 --pretty=format:%H -- ${targetDir}`).toString().trim();

    if (!latestCommitHash) {
        console.error(`No commits found for the directory: ${targetDir}`);
        process.exit(1);
    }

} catch (error) {
    console.error(`Error executing git commands: ${error.message}`);
    process.exit(1);
}

console.log(`Running git diff from commit ${latestCommitHash} to HEAD for ${diffPath}...`);
const gitDiff = spawn("git", ["diff", latestCommitHash, "HEAD", "--", diffPath], { stdio: "inherit" });

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
