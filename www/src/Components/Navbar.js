import React from 'react';
import { NavLink } from "react-router-dom";
import './Navbar.scss';

function Navbar(props) {
	return (
		<nav className="navbar navbar-expand-md navbar-dark fixed-top bg-dark">
			<div className="container-fluid navbar-content">
				<NavLink className="navbar-brand" to="/">
					<img src="images/logo.png" className="title-logo" alt="logo" />GP2040
				</NavLink>
				<button className="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarCollapse" aria-controls="navbarCollapse" aria-expanded="false" aria-label="Toggle navigation">
					<span className="navbar-toggler-icon"></span>
				</button>
				<div className="collapse navbar-collapse" id="navbarCollapse">
					<ul className="navbar-nav me-auto mb-2 mb-md-0">
						<li className="nav-item">
							<NavLink exact={true} className="nav-link" activeClassName="active" to="/">Home</NavLink >
						</li>
						<li className="nav-item">
							<NavLink exact={true} className="nav-link" activeClassName="active" to="/pin-mapping">Pin Mapping</NavLink >
						</li>
						<li className="nav-item dropdown">
							<a className="nav-link dropdown-toggle" href="#" role="button" data-bs-toggle="dropdown">Links</a>
							<ul className="dropdown-menu">
								<li><a className="dropdown-item" target="_blank" rel="noreferrer" href="https://github.com/FeralAI/GP2040/">Home Page</a></li>
								<li><a className="dropdown-item" target="_blank" rel="noreferrer" href="https://github.com/FeralAI/GP2040/releases">Releases</a></li>
							</ul>
						</li>
					</ul>
				</div>
			</div>
		</nav>
	);
}

export default Navbar;
