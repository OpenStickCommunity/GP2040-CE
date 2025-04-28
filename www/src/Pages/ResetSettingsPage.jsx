import { Trans, useTranslation } from 'react-i18next';

import DangerSection from '../Components/DangerSection';
import WebApi from '../Services/WebApi';

export default function ResetSettingsPage() {
	const { t } = useTranslation('');

	const resetSettings = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		if (window.confirm(t('ResetSettings:confirm-text'))) {
			const result = await WebApi.resetSettings();
			console.log(result);
			setTimeout(() => {
				window.location.reload();
			}, 2000);
		}
	};

	return (
		<DangerSection title={t('ResetSettings:header-text')}>
			<Trans ns="ResetSettings" i18nKey="sub-header-text">
				<p className="card-text">
					This option resets all saved configurations on your controller. Use
					this option as a last resort or when trying to diagnose odd issues
					with your controller.
				</p>
				<p className="card-text">
					This process will automatically reset the controller.
				</p>
			</Trans>
			<button className="btn btn-danger" onClick={resetSettings}>
				{t('Common:button-reset-settings-label')}
			</button>
		</DangerSection>
	);
}
