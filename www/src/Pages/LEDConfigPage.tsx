import { useContext, useEffect, useState } from 'react';
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import Row from 'react-bootstrap/Row';

import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { useTranslation } from 'react-i18next';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import WebApi from '../Services/WebApi';
import { FormCheck } from 'react-bootstrap';

const LED_FORMATS = [
	{ label: 'GRB', value: 0 },
	{ label: 'RGB', value: 1 },
	{ label: 'GRBW', value: 2 },
	{ label: 'RGBW', value: 3 },
];

const defaultValue = {
	dataPin: -1,
	ledFormat: 0,
	turnOffWhenSuspended: 0,
	brightnessMaximum: 0,
};

const schema = yup.object().shape({
	dataPin: yup.number().required().checkUsedPins(),
	ledFormat: yup
		.number()
		.required()
		.positive()
		.integer()
		.min(0)
		.max(3)
		.label('LED Format'),
	brightnessMaximum: yup
		.number()
		.required()
		.positive()
		.integer()
		.min(0)
		.max(100)
		.label('Max Brightness'),
	turnOffWhenSuspended: yup.number().label('Turn Off When Suspended'),
});

const FormContext = () => {
	const { setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getLedOptions();
			setValues(data);
		}
		fetchData();
	}, [setValues]);
	return null;
};

export default function LEDConfigPage() {
	const { updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { t } = useTranslation('');

	const onSuccess = async (values: typeof defaultValue) => {
		const success = await WebApi.setLedOptions(values);
		if (success) updateUsedPins();

		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	return (
		<Formik
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={defaultValue}
			validateOnChange={true}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<Form onSubmit={handleSubmit}>
					<Section title={t('LedConfig:rgb.header-text')}>
						<Row>
							<FormControl
								type="number"
								label={t('LedConfig:rgb.data-pin-label')}
								name="dataPin"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.dataPin}
								error={errors.dataPin}
								isInvalid={Boolean(errors.dataPin)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label={t('LedConfig:rgb.led-format-label')}
								name="ledFormat"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledFormat}
								error={errors.ledFormat}
								isInvalid={Boolean(errors.ledFormat)}
								onChange={(e) =>
									setFieldValue('ledFormat', parseInt(e.target.value))
								}
							>
								{LED_FORMATS.map((o, i) => (
									<option key={`ledFormat-option-${i}`} value={o.value}>
										{o.label}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('LedConfig:rgb.led-brightness-maximum-label')}
								name="brightnessMaximum"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.brightnessMaximum}
								error={errors.brightnessMaximum}
								isInvalid={Boolean(errors.brightnessMaximum)}
								onChange={handleChange}
								min={0}
								max={100}
							/>
						</Row>
						<Row>
							<div className="col-sm-4 mb-3">
								<FormCheck
									label={t('LedConfig:turn-off-when-suspended')}
									type="switch"
									id="turnOffWhenSuspended"
									isInvalid={false}
									checked={Boolean(values.turnOffWhenSuspended)}
									onChange={(e) => {
										setFieldValue(
											'turnOffWhenSuspended',
											e.target.checked ? 1 : 0,
										);
									}}
								/>
							</div>
						</Row>
					</Section>
					<Button type="submit">{t('Common:button-save-label')}</Button>
					{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					<FormContext />
				</Form>
			)}
		</Formik>
	);
}
