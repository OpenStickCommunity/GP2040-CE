import { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, FormCheck, FormSelect, Table } from 'react-bootstrap';
import { Formik, useFormikContext, getIn } from 'formik';
import * as yup from 'yup';
import { useTranslation } from 'react-i18next';
import ContextualHelpOverlay from '../Components/ContextualHelpOverlay';

import Section from '../Components/Section';
import WebApi, { basePeripheralMapping } from '../Services/WebApi';
import { PERIPHERAL_DEVICES } from '../Data/Peripherals';
import boards from '../Data/Boards.json';

let peripheralFieldsSchema = {
	peripheral: yup.object().shape(
		Object.assign(
			{},
			...PERIPHERAL_DEVICES.map((device) => {
				let deviceProps = Object.assign(
					{},
					...device.blocks.map((block) => {
						return {
							[block.label]: yup.object().shape(
								Object.assign(
									{ enabled: yup.boolean().label(`${block.label} Enabled`) },
									//...Array.from(Object.keys(block.pins), (pin) => ({[pin]: yup.number().label(`${pin} Pin`).validatePinWhenValue(`peripheral.${block.label}.enabled`)}) ),
									...Array.from(Object.keys(block.pins), (pin) => ({
										[pin]: yup.number().label(`${pin} Pin`),
									})),
									...Array.from(Object.keys(device.options), (opt) => ({
										[opt]: yup.number().label('${block.label} ${opt} Setting'),
									})),
								),
							),
						};
					}),
				);
				return deviceProps;
			}),
		),
	),
};

const schema = yup.object().shape({
	...peripheralFieldsSchema,
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			await WebApi.getGamepadOptions(setLoading);
			const peripheralOptions = await WebApi.getPeripheralOptions(setLoading);

			setValues(peripheralOptions);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {}, [values, setValues]);

	return null;
};

export default function PeripheralMappingPage() {
	const { setButtonLabels, usedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	let allPins = [
		...Array(boards[import.meta.env.VITE_GP2040_BOARD].maxPin + 1).keys(),
	];
	const pinLookup = (pinList) => {
		return pinList && pinList.length > 0 ? pinList : allPins;
	};

	const onSuccess = async (values) => {
		const cleanValues = schema.cast(values);
		console.dir(cleanValues);

		const success = await WebApi.setPeripheralOptions(cleanValues);

		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const generatePeripheralDetails = (header, peripheral) => {
		return (
			<div key={`details-${peripheral.value}-${peripheral.label}`}>
				<div key={`details-${peripheral.value}-header`} className="mb-3">
					{header}
				</div>
				{peripheral.pinTable &&
					peripheral.blocks.map((block, i) => {
						let colCount = Math.max.apply(
							null,
							Object.keys(block.pins).map((pin) => block.pins[pin].length),
						);
						return (
							<Table
								key={`details-${i}`}
								className="caption-top"
								striped="columns"
								responsive
								bordered
								hover
								variant="dark"
								size="sm"
							>
								<caption>{block.label.toUpperCase()}</caption>
								<tbody>
									<tr>
										<th scope="row"></th>
										<th colSpan={colCount}>
											{t('PeripheralMapping:pins-label')}
										</th>
									</tr>
									{Object.keys(block.pins).map((pinName) => (
										<tr key={`block-info-${pinName}`}>
											<th scope="row" className="col-2">
												{pinName.toUpperCase()}
											</th>
											{block.pins[pinName].map((pin) => (
												<td key={`block-info-${pinName}-${pin}`}>{pin}</td>
											))}
											{block.pins[pinName].length < colCount ? <td></td> : ''}
										</tr>
									))}
								</tbody>
							</Table>
						);
					})}
			</div>
		);
	};

	const { t } = useTranslation('');

	return (
		<Formik
			onSubmit={onSuccess}
			validationSchema={schema}
			initialValues={basePeripheralMapping}
		>
			{({ errors, handleSubmit, setFieldValue, values }) =>
				console.log('errors', errors) || (
					<div>
						<Form noValidate onSubmit={handleSubmit}>
							<Section title={t('PeripheralMapping:header-text')}>
								<p>{t('PeripheralMapping:sub-header-text')}</p>
								{PERIPHERAL_DEVICES.map((peripheral, i) => (
									<Form.Group
										key={`peripheral-${peripheral.value}`}
										className="row mb-3"
									>
										<Form.Label>
											{t(`PeripheralMapping:${peripheral.label}-label`)}
											<ContextualHelpOverlay
												title={t(
													`PeripheralMapping:${peripheral.label}-desc-header`,
												)}
												body={generatePeripheralDetails(
													t(
														`PeripheralMapping:${peripheral.label}-description`,
													),
													peripheral,
												)}
											></ContextualHelpOverlay>
										</Form.Label>
										{peripheral.blocks.map((block, i) => (
											<div
												key={`peripheral${peripheral.value}block${block.value}`}
												className="row mb-3"
											>
												<div className="col-sm-auto">
													<FormCheck
														key={`peripheral.${block.label}.enabled`}
														name={`peripheral.${block.label}.enabled`}
														label={`${block.label.toUpperCase()}`}
														id={`peripheral.${block.label}.enabled`}
														type="switch"
														reverse={true}
														isInvalid={false}
														className="form-select-sm"
														value={values.peripheral[`${block.label}`].enabled}
														checked={Boolean(
															values.peripheral[`${block.label}`].enabled,
														)}
														onChange={(e) => {
															setFieldValue(
																`peripheral.${block.label}.enabled`,
																e.target.checked ? 1 : 0,
															);
														}}
													/>
												</div>
												{Object.keys(block.pins).map((pin, i) => (
													<div
														key={`${block.label}.${pin}`}
														className="col-sm-auto"
													>
														<Form.Label>
															{t(
																`PeripheralMapping:pin-${pin.toLowerCase()}-label`,
															)}
														</Form.Label>
														<FormSelect
															key={`peripheral.${block.label}.${pin}`}
															id={`peripheral.${block.label}.${pin}`}
															name={`peripheral.${block.label}.${pin}`}
															className="form-select-sm sm-1"
															disabled={
																!Boolean(
																	values.peripheral[`${block.label}`].enabled,
																)
															}
															error={getIn(
																errors,
																`peripheral.${block.label}.${pin}`,
															)}
															value={
																values.peripheral[`${block.label}`][`${pin}`]
															}
															onChange={(e) => {
																setFieldValue(
																	`peripheral.${block.label}.${pin}`,
																	e.target.value,
																);
															}}
														>
															<option
																key={`block-${block.label}-pin-unset`}
																value="-1"
															>
																Unset
															</option>
															{pinLookup(block.pins[pin]).map((o, i2) => (
																<option
																	key={`block-${block.label}-pin-${i2}`}
																	value={o}
																>
																	{!usedPins.includes(o)
																		? o
																		: `${o} - ${t(
																				'PeripheralMapping:pin-in-use',
																			)}`}
																</option>
															))}
														</FormSelect>
													</div>
												))}
												{Object.keys(peripheral.options).map((option, i) => (
													<div
														key={`${block.label}.${option}`}
														className="col-sm-auto"
													>
														<Form.Label>
															{t(
																`PeripheralMapping:option-${option.toLowerCase()}-label`,
															)}
														</Form.Label>
														<FormSelect
															key={`peripheral.${block.label}.${option}`}
															id={`peripheral.${block.label}.${option}`}
															name={`peripheral.${block.label}.${option}`}
															className="form-select-sm sm-1"
															disabled={
																!Boolean(
																	values.peripheral[`${block.label}`].enabled,
																)
															}
															error={getIn(
																errors,
																`peripheral.${block.label}.${option}`,
															)}
															value={
																values.peripheral[`${block.label}`][`${option}`]
															}
															onChange={(e) => {
																setFieldValue(
																	`peripheral.${block.label}.${option}`,
																	e.target.value,
																);
															}}
														>
															{peripheral.options[option].map((o, i2) => (
																<option
																	key={`block-${block.label}-option-${option}-${o.value}`}
																	value={o.value}
																>
																	{`${t(
																		`PeripheralMapping:option-${option}-choice-${o.value}-label`,
																	)} - ${o.value}`}
																</option>
															))}
														</FormSelect>
													</div>
												))}
											</div>
										))}
									</Form.Group>
								))}
							</Section>
							<Button type="submit">{t('Common:button-save-label')}</Button>
							{saveMessage ? (
								<span className="alert">{saveMessage}</span>
							) : null}
							<FormContext />
						</Form>
					</div>
				)
			}
		</Formik>
	);
}
