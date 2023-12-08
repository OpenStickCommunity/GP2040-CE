import React from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

// TODO: add scheme for validation
export const snesScheme = {};

export const snesState = {
	SNESpadAddonEnabled: 0,
	snesPadClockPin: -1,
	snesPadLatchPin: -1,
	snesPadDataPin: -1,
};

const SNES = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:snes-extension-header-text')}>
			<div id="SNESpadAddonOptions" hidden={!values.SNESpadAddonEnabled}>
				<Row>
					<Trans ns="AddonsConfig" i18nKey="snes-extension-sub-header-text">
						<p>
							Note: If the Display is enabled at the same time, this Addon will
							be disabled.
						</p>
						<h3>Currently Supported Controllers</h3>
						<p>
							SNES pad: D-Pad Supported. B = B1, A = B2, Y = B3, X = B4, L = L1,
							R = R1, Select = S1, Start = S2
						</p>
						<p>
							SNES mouse: Analog Stick Supported. Left Click = B1, Right Click =
							B2
						</p>
						<p>NES: D-Pad Supported. B = B1, A = B2, Select = S1, Start = S2</p>
					</Trans>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:snes-extension-clock-pin-label')}
						name="snesPadClockPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.snesPadClockPin}
						error={errors.snesPadClockPin}
						isInvalid={errors.snesPadClockPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:snes-extension-latch-pin-label')}
						name="snesPadLatchPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.snesPadLatchPin}
						error={errors.snesPadLatchPin}
						isInvalid={errors.snesPadLatchPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:snes-extension-data-pin-label')}
						name="snesPadDataPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.snesPadDataPin}
						error={errors.snesPadDataPin}
						isInvalid={errors.snesPadDataPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="SNESpadButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.SNESpadAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('SNESpadAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default SNES;
