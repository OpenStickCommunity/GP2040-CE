import React from 'react';
import DangerSection from '../Components/DangerSection';
import WebApi from '../Services/WebApi';

export default function ResetSettingsPage() {

	const resetSettings = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		if (window.confirm('Are you sure you want to reset saved configuration?')) {
			const result = await WebApi.resetSettings();
			console.log(result);
			setTimeout(() => {
				window.location.reload();
			}, 2000);
		}
	};

	return (
		<DangerSection title="Reset Settings">
			<p className="card-text">
				This option resets all saved configurations on your controller. Use this option as a
				last resort or when trying to diagnose odd issues with your controller.
			</p>
			<p className="card-text">This process will automatically reset the controller.</p>
			<button className="btn btn-danger" onClick={resetSettings}>Reset Settings</button>
		</DangerSection>
	);
}
