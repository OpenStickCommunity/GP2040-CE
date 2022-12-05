import React, { useState } from 'react';
import { BrowserRouter as Router, Route, Switch } from "react-router-dom";
import { AppContext } from './Contexts/AppContext';
import Navigation from './Components/Navigation'

import HomePage from './Pages/HomePage'
import PinMappingPage from "./Pages/PinMapping";
import ResetSettingsPage from './Pages/ResetSettingsPage';
import SettingsPage from './Pages/SettingsPage';
import DisplayConfigPage from './Pages/DisplayConfig';
import BuzzerConfigPage from './Pages/BuzzerConfig';
import LEDConfigPage from './Pages/LEDConfigPage';
import AddonsConfigPage from './Pages/AddonsConfigPage';
import BackupPage from './Pages/BackupPage';

import { loadButtonLabels } from './Services/Storage';
import './App.scss';

const App = () => {
	const [buttonLabels, setButtonLabels] = useState(loadButtonLabels() ?? 'gp2040');

	const appData = {
		buttonLabels,
		setButtonLabels,
	};

	return (
		<AppContext.Provider value={appData}>
			<Router>
				<Navigation />
				<div className="container-fluid body-content">
					<Switch>
						<Route exact path="/">
							<HomePage />
						</Route>
						<Route path="/settings">
							<SettingsPage />
						</Route>
						<Route path="/pin-mapping">
							<PinMappingPage />
						</Route>
						<Route path="/reset-settings">
							<ResetSettingsPage />
						</Route>
						<Route path="/led-config">
							<LEDConfigPage />
						</Route>
						<Route path="/display-config">
							<DisplayConfigPage />
						</Route>
						<Route path="/buzzer-config">
							<BuzzerConfigPage />
						</Route>
						<Route path="/add-ons">
							<AddonsConfigPage />
						</Route>
						<Route path="/backup">
							<BackupPage />
						</Route>
					</Switch>
				</div>
			</Router>
		</AppContext.Provider>
	);
}

export default App;
