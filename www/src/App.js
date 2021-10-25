import React from 'react';
import { BrowserRouter as Router, Route, Switch } from "react-router-dom";
import './App.scss';

import Navbar from './Components/Navbar'
import HomePage from './Pages/HomePage'
import PinMappingPage from "./Pages/PinMapping";

function App() {
	return (
		<Router>
			<Navbar />
			<div className="container-fluid body-content">
				<Switch>
					<Route exact path="/">
						<HomePage />
					</Route>
					<Route path="/pin-mapping">
						<PinMappingPage />
					</Route>
				</Switch>
			</div>
		</Router>
	);
}

export default App;
