import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import path from "path";

// https://vitejs.dev/config/
export default defineConfig({
	build: {
		outDir: path.join(__dirname, "build"),
		sourcemap: false,
	},
	server: {
		open: true,
		port: 3000,
	},
	plugins: [react()],
	resolve: {
		alias: {
			"~bootstrap": path.resolve(__dirname, "node_modules/bootstrap"),
		},
	},
});
