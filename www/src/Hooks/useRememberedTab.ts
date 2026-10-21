import { useState } from 'react';
import { useLocation } from 'react-router-dom';

const rememberedTabs = new Map<string, { locationKey: string; key: string }>();

export const useRememberedTab = (tabsId: string, defaultKey: string) => {
	const { key: locationKey } = useLocation();
	const [activeKey, setActiveKey] = useState(() => {
		const remembered = rememberedTabs.get(tabsId);
		return remembered?.locationKey === locationKey
			? remembered.key
			: defaultKey;
	});

	const selectTab = (key: string | null) => {
		if (key === null) return;
		rememberedTabs.set(tabsId, { locationKey, key });
		setActiveKey(key);
	};

	return [activeKey, selectTab] as const;
};
