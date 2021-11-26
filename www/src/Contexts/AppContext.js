import { createContext } from 'react';

export const defaultAppData = {
	buttonLabels: 'gp2040',
};

export const AppContext = createContext(defaultAppData);
