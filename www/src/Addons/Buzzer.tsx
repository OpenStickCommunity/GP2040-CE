import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const buzzerScheme = {
	BuzzerSpeakerAddonEnabled: yup
		.number()
		.required()
		.label('Buzzer Speaker Add-On Enabled'),
	buzzerPin: yup
		.number()
		.label('Buzzer Pin')
		.validatePinWhenValue('BuzzerSpeakerAddonEnabled'),
	buzzerEnablePin: yup
		.number()
		.label('Buzzer Enable Pin')
		.validatePinWhenValue('BuzzerSpeakerAddonEnabled'),
	buzzerVolume: yup
		.number()
		.label('Buzzer Volume')
		.validateRangeWhenValue('BuzzerSpeakerAddonEnabled', 0, 100),
};

export const buzzerState = {
	BuzzerSpeakerAddonEnabled: 0,
	buzzerPin: -1,
	buzzerEnablePin: -1,
	buzzerVolume: 100,
};

const Buzzer = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/buzzer-speaker"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:buzzer-speaker-header-text')}
			</a>
		}
		>
			<div
				id="BuzzerSpeakerAddonOptions"
				hidden={!values.BuzzerSpeakerAddonEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:buzzer-speaker-pin-label')}
						name="buzzerPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.buzzerPin}
						error={errors.buzzerPin}
						isInvalid={Boolean(errors.buzzerPin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:buzzer-speaker-enable-pin-label')}
						name="buzzerEnablePin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.buzzerEnablePin}
						error={errors.buzzerEnablePin}
						isInvalid={Boolean(errors.buzzerEnablePin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:buzzer-speaker-volume-label')}
						name="buzzerVolume"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.buzzerVolume}
						error={errors.buzzerVolume}
						isInvalid={Boolean(errors.buzzerVolume)}
						onChange={handleChange}
						min={0}
						max={100}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="BuzzerSpeakerAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.BuzzerSpeakerAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('BuzzerSpeakerAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Buzzer;
