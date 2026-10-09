// Checks the web preview Rapid Trigger logic against the firmware test vectors.
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { build } from "esbuild";

const root = fileURLToPath(new URL("../..", import.meta.url));

const { outputFiles } = await build({
	entryPoints: [`${root}/www/src/Addons/HERapidTrigger.ts`],
	bundle: true,
	format: "esm",
	write: false,
});
const { createRapidTriggerState, updateRapidTrigger } = await import(
	`data:text/javascript;base64,${Buffer.from(outputFiles[0].text).toString("base64")}`
);

const lines = readFileSync(`${root}/tests/he_rapid_trigger/vectors.txt`, "utf8")
	.split("\n")
	.filter((line) => line.trim() && !line.startsWith("#"));

let failures = 0;
for (const line of lines) {
	const [name, configText, depthsText, expected] = line
		.split("|")
		.map((part) => part.trim());
	const [rapid, continuous, actuation, press, release, noise, travel] =
		configText.split(/\s+/).map(Number);
	const config = {
		actuation,
		pressSensitivity: press,
		releaseSensitivity: release,
		noise,
		travel,
		rapidTrigger: Boolean(rapid),
		continuous: Boolean(continuous),
	};
	const state = createRapidTriggerState();
	const actual = depthsText
		.split(/\s+/)
		.map((depth) =>
			updateRapidTrigger(state, config, Number(depth)) ? "1" : "0",
		)
		.join("");
	if (actual !== expected) {
		failures++;
		console.log(`FAIL ${name}: expected ${expected} got ${actual}`);
	}
}

console.log(
	`${lines.length - failures}/${lines.length} rapid trigger vectors passed`,
);
process.exit(failures ? 1 : 0);
