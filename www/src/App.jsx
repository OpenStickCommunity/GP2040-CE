import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';

import { AppContextProvider } from './Contexts/AppContext';

import Navigation from './Components/Navigation';

import HomePage from './Pages/HomePage';
import PinMappingPage from './Pages/PinMapping';
import PeripheralMappingPage from './Pages/PeripheralMappingPage';
import ResetSettingsPage from './Pages/ResetSettingsPage';
import SettingsPage from './Pages/SettingsPage';
import DisplayConfigPage from './Pages/DisplayConfig';
import LEDConfigPage from './Pages/LEDConfigPage';
import CustomThemePage from './Pages/CustomThemePage';
import AddonsConfigPage from './Pages/AddonsConfigPage';
import BackupPage from './Pages/BackupPage';
import PlaygroundPage from './Pages/PlaygroundPage';
import InputMacroAddonPage from './Pages/InputMacroAddonPage';

import './App.scss';

const App = () => {
	return (
		<AppContextProvider>
			<Router>
				<Navigation />
				<div className="body-content container-lg">
					<Routes>
						<Route path="/" element={<HomePage />} />
						<Route path="/settings" element={<SettingsPage />} />
						<Route path="/pin-mapping" element={<PinMappingPage />} />
						<Route
							path="/peripheral-mapping"
							element={<PeripheralMappingPage />}
						/>
						<Route path="/reset-settings" element={<ResetSettingsPage />} />
						<Route path="/led-config" element={<LEDConfigPage />} />
						<Route path="/custom-theme" element={<CustomThemePage />} />
						<Route path="/display-config" element={<DisplayConfigPage />} />
						<Route path="/add-ons" element={<AddonsConfigPage />} />
						<Route path="/backup" element={<BackupPage />} />
						<Route path="/playground" element={<PlaygroundPage />} />
						<Route path="/macro" element={<InputMacroAddonPage />} />
					</Routes>
				</div>
			</Router>
		</AppContextProvider>
	);
};

export default App;
