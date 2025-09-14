import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';

import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const tg16Scheme = {};

export const tg16State = {
	TG16padAddonEnabled: 0,
	tg16PadOePin: -1,
	tg16PadSelectPin: -1,
	tg16PadDataPin0: -1,
	tg16PadDataPin1: -1,
	tg16PadDataPin2: -1,
	tg16PadDataPin3: -1,
};

const TG16 = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/tg16-input"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:tg16-extension-header-text')}
			</a>
		}
		>
			<div id="TG16padAddonOptions" hidden={!values.TG16padAddonEnabled}>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:tg16-extension-sub-header-text')}
				</div>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:tg16-extension-desc-header')}
				</div>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-oe-pin-label')}
						name="tg16PadOePin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadOePin}
						error={errors.tg16PadOePin}
						isInvalid={Boolean(errors.tg16PadOePin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-select-pin-label')}
						name="tg16PadSelectPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadSelectPin}
						error={errors.tg16PadSelectPin}
						isInvalid={Boolean(errors.tg16PadSelectPin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-data-pin0-label')}
						name="tg16PadDataPin0"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadDataPin0}
						error={errors.tg16PadDataPin0}
						isInvalid={Boolean(errors.tg16PadDataPin0)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-data-pin1-label')}
						name="tg16PadDataPin1"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadDataPin1}
						error={errors.tg16PadDataPin1}
						isInvalid={Boolean(errors.tg16PadDataPin1)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-data-pin2-label')}
						name="tg16PadDataPin2"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadDataPin2}
						error={errors.tg16PadDataPin2}
						isInvalid={Boolean(errors.tg16PadDataPin2)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tg16-extension-data-pin3-label')}
						name="tg16PadDataPin3"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tg16PadDataPin3}
						error={errors.tg16PadDataPin3}
						isInvalid={Boolean(errors.tg16PadDataPin3)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="TG16padButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.TG16padAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('TG16padAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default TG16;
