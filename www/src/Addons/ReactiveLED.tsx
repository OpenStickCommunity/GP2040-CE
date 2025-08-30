import { AppContext } from '../Contexts/AppContext';
import { useContext, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import invert from 'lodash/invert';
import omit from 'lodash/omit';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { Button, FormCheck, Row } from 'react-bootstrap';

import WebApi from '../Services/WebApi';

import { BUTTON_ACTIONS } from '../Data/Pins';
import { getButtonLabels } from '../Data/Buttons';

import './ReactiveLED.scss';

const NON_SELECTABLE_BUTTON_ACTIONS = [
	-5, 0, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39, 40,
];

const REACTIVE_LED_MODES = [
	{ label: 'static-off', value: 0 },
	{ label: 'static-on', value: 1 },
	{ label: 'fade-in', value: 2 },
	{ label: 'fade-out', value: 3 },
];

export const reactiveLEDScheme = {
	ReactiveLEDAddonEnabled: yup
		.number()
		.required()
		.label('Reactive LED Add-On Enabled'),
};

export const reactiveLEDState = {
	ReactiveLEDAddonEnabled: 0,
};

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const getOption = (o, actionId) => {
	return {
		label: invert(BUTTON_ACTIONS)[actionId],
		value: actionId,
	};
};

const ReactiveLED = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	const [reactiveLEDs, setSelectedLEDs] = useState([]);
	const [selectedLEDs] = useState([]);
	const { setLoading, buttonLabels } = useContext(AppContext);

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	useEffect(() => {
		async function fetchData() {
			const ledSettings = await WebApi.getReactiveLEDs(setLoading);
			setSelectedLEDs(ledSettings.leds);
		}

		fetchData();
	}, [setSelectedLEDs, selectedLEDs]);

	const handleLEDChange = (
		c: any,
		idx: number,
		field: string,
		value: number,
	) => {
		c[idx][field] = value;
		console.dir(c);
		return [...c];
	};

	const saveLEDSettings = async () => {
		await WebApi.setReactiveLEDs({ leds: reactiveLEDs });
	};

	return (
		<Section title={t('ReactiveLED:header-text')}>
			<div
				id="ReactiveLEDEnabledOptions"
				hidden={!values.ReactiveLEDAddonEnabled}
			>
				<div className="reactive-led-grid gap-3">
					{reactiveLEDs?.length > 0 &&
						reactiveLEDs.map((o, i) => (
							<div className="align-items-center gap-2" key={`reactiveLED${i}`}>
								<h5>{t('ReactiveLED:led-panel-label', { index: i })}</h5>
								<FormControl
									type="number"
									label={t('ReactiveLED:led-pin-label')}
									name={`led${i}Pin`}
									className="form-control-sm"
									groupClassName="mb-3"
									value={reactiveLEDs[i].pin}
									onChange={(e) =>
										setSelectedLEDs((c) =>
											handleLEDChange(c, i, 'pin', Number(e.target.value)),
										)
									}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t('ReactiveLED:led-action-label')}
									name={`led${i}Action`}
									className="form-select-sm"
									groupClassName="mb-3"
									value={reactiveLEDs[i].action}
									onChange={(e) =>
										setSelectedLEDs((c) =>
											handleLEDChange(c, i, 'action', Number(e.target.value)),
										)
									}
								>
									{Object.values(BUTTON_ACTIONS)
										.filter((o) => !isNonSelectable(o))
										.map((mo, mi) => {
											const opt = getOption(null, mo);
											const labelKey = opt.label.split('BUTTON_PRESS_').pop();
											return (
												<option key={`led${i}Action-option-${mi}`} value={mo}>
													{(labelKey && buttonNames[labelKey]) ||
														t(`PinMapping:actions.${opt.label}`)}
												</option>
											);
										})}
								</FormSelect>
								<FormSelect
									label={t('ReactiveLED:led-mode-up-label')}
									name={`led${i}ModeUp`}
									className="form-select-sm"
									groupClassName="mb-3"
									value={reactiveLEDs[i].modeUp}
									onChange={(e) =>
										setSelectedLEDs((c) =>
											handleLEDChange(c, i, 'modeUp', Number(e.target.value)),
										)
									}
								>
									{REACTIVE_LED_MODES.map((mo, mi) => (
										<option key={`led${i}ModeUp-option-${mi}`} value={mo.value}>
											{t(`ReactiveLED:led-mode.${mo.label}`)}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('ReactiveLED:led-mode-down-label')}
									name={`led${i}ModeDown`}
									className="form-select-sm"
									groupClassName="mb-3"
									value={reactiveLEDs[i].modeDown}
									onChange={(e) =>
										setSelectedLEDs((c) =>
											handleLEDChange(c, i, 'modeDown', Number(e.target.value)),
										)
									}
								>
									{REACTIVE_LED_MODES.map((mo, mi) => (
										<option
											key={`led${i}ModeDown-option-${mi}`}
											value={mo.value}
										>
											{t(`ReactiveLED:led-mode.${mo.label}`)}
										</option>
									))}
								</FormSelect>
							</div>
						))}
				</div>
				<Row className="mb-3">
					<div className="col-sm-3 mb-3">
						<Button type="button" onClick={saveLEDSettings}>
							{t('ReactiveLED:button-save')}
						</Button>
					</div>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="ReactiveLEDAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ReactiveLEDAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('ReactiveLEDAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ReactiveLED;
