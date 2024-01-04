/// <reference types="vite/client" />

interface ImportMetaEnv {
	readonly VITE_GP2040_BOARD: string;
	readonly VITE_GP2040_CONTROLLER: string;
	readonly VITE_DEV_BASE_URL: string;
	readonly VITE_DEV_HOST: string;
}

interface ImportMeta {
	readonly env: ImportMetaEnv;
}
