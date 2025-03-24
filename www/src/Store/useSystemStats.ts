import { create } from 'zustand';
import { baseUrl } from '../Services/WebApi';

const percentage = (x: number, y: number): number =>
	parseFloat(((x / y) * 100).toFixed(2));
const toKB = (x: number): number => parseFloat((x / 1024).toFixed(2));

type State = {
	latestVersion: string;
	latestDownloadUrl: string;
	currentVersion: string;
	boardConfigProperties: {
		label: string;
		fileName: string;
	};
	memoryReport: {
		percentageFlash: number;
		percentageHeap: number;
		physicalFlash: number;
		staticAllocs: number;
		totalFlash: number;
		totalHeap: number;
		usedFlash: number;
		usedHeap: number;
	};
	loading: boolean;
	error: boolean;
};

type Actions = {
	getSystemStats: () => void;
};

const INITIAL_STATE: State = {
	latestVersion: '',
	latestDownloadUrl: '',
	currentVersion: '',
	boardConfigProperties: {
		label: '',
		fileName: '',
	},
	memoryReport: {
		percentageFlash: 0,
		percentageHeap: 0,
		physicalFlash: 0,
		staticAllocs: 0,
		totalFlash: 0,
		totalHeap: 0,
		usedFlash: 0,
		usedHeap: 0,
	},
	loading: false,
	error: false,
};

const useSystemStats = create<State & Actions>()((set) => ({
	...INITIAL_STATE,
	getSystemStats: async () => {
		set({ loading: true });

		try {
			const [firmwareVersion, memoryReport, latestRelease] = await Promise.all([
				fetch(`${baseUrl}/api/getFirmwareVersion`).then((res) => res.json()),
				fetch(`${baseUrl}/api/getMemoryReport`).then((res) => res.json()),
				fetch(
					'https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases/latest',
				).then((res) => res.json()),
			]);
			const latestDownloadUrl =
				latestRelease.assets?.find(
					({ name }: { name: string }) =>
						name
							?.substring(name.lastIndexOf('_') + 1)
							?.replace('.uf2', '')
							?.toLowerCase() === firmwareVersion.boardConfig.toLowerCase(),
				)?.browser_download_url ||
				`https://github.com/OpenStickCommunity/GP2040-CE/releases/tag/${latestRelease.tag_name}`;

			set({
				currentVersion: firmwareVersion.version,
				latestVersion: latestRelease.tag_name,
				latestDownloadUrl,
				boardConfigProperties: {
					label: firmwareVersion.boardConfigLabel,
					fileName: firmwareVersion.boardConfigFileName,
				},
				memoryReport: {
					physicalFlash: toKB(memoryReport.physicalFlash),
					staticAllocs: toKB(memoryReport.staticAllocs),
					totalFlash: toKB(memoryReport.totalFlash),
					totalHeap: toKB(memoryReport.totalHeap),
					usedFlash: toKB(memoryReport.usedFlash),
					usedHeap: toKB(memoryReport.usedHeap),
					percentageFlash: percentage(
						memoryReport.usedFlash,
						memoryReport.totalFlash,
					),
					percentageHeap: percentage(
						memoryReport.usedHeap,
						memoryReport.totalHeap,
					),
				},
				loading: false,
			});
		} catch (error) {
			set({ error: true, loading: false });
		}
	},
}));

export default useSystemStats;
