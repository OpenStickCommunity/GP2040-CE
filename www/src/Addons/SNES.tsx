import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import ContextualHelpOverlay from '../Components/ContextualHelpOverlay';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

// TODO: add scheme for validation
export const snesScheme = {};

export const snesState = {
	SNESpadAddonEnabled: 0,
	snesPadClockPin: -1,
	snesPadLatchPin: -1,
	snesPadDataPin: -1,
};

const SNES_MAPPING_TABLE = [
	{ gp: 'Up',		nes: 'Up',		snes: 'Up',		mouse: '' },
	{ gp: 'Down',	nes: 'Down',	snes: 'Down',	mouse: '' },
	{ gp: 'Left',	nes: 'Left',	snes: 'Left',	mouse: '' },
	{ gp: 'Right',	nes: 'Right',	snes: 'Right',	mouse: '' },
	{ gp: 'B1',		nes: 'B',		snes: 'B',		mouse: 'Left Click' },
	{ gp: 'B2',		nes: 'A',		snes: 'A',		mouse: 'Right Click' },
	{ gp: 'B3',		nes: '',		snes: 'Y',		mouse: '' },
	{ gp: 'B4',		nes: '',		snes: 'X',		mouse: '' },
	{ gp: 'L1',		nes: '',		snes: 'L',		mouse: '' },
	{ gp: 'R1',		nes: '',		snes: 'R',		mouse: '' },
	{ gp: 'S1',		nes: 'Select',	snes: 'Select',	mouse: '' },
	{ gp: 'S2',		nes: 'Start',	snes: 'Start',	mouse: '' },
	{ gp: 'Analog',	nes: '',		snes: '',		mouse: 'Mouse Movement' },
];

const SNES = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/snes-input"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:snes-extension-header-text')}
			</a>
		}
		>
			<div id="SNESpadAddonOptions" hidden={!values.SNESpadAddonEnabled}>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:snes-extension-sub-header-text')}
				</div>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:snes-extension-desc-header')}
						<ContextualHelpOverlay
							title={t('AddonsConfig:snes-extension-desc-header')}
							body={
								<table className="caption-top table table-dark table-sm table-striped-columns table-bordered table-hover">
									<thead>
										<tr>
										<th>GP2040</th>
										<th>NES</th>
										<th>SNES</th>
										<th>Super NES Mouse</th>
										</tr>
									</thead>
									<tbody>
										{SNES_MAPPING_TABLE.map(({gp, nes, snes, mouse}, i) =>
											<tr key={i}>
												<td><code>{gp}</code></td>
												<td>{nes}</td>
												<td>{snes}</td>
												<td>{mouse}</td>
											</tr>
										)}
									</tbody>
								</table>
							}
						/>
				</div>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:snes-extension-clock-pin-label')}
						name="snesPadClockPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.snesPadClockPin}
						error={errors.snesPadClockPin}
						isInvalid={Boolean(errors.snesPadClockPin)}
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
						isInvalid={Boolean(errors.snesPadLatchPin)}
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
						isInvalid={Boolean(errors.snesPadDataPin)}
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
					handleCheckbox('SNESpadAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default SNES;
