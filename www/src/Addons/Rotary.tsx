import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { FormCheck, Row, Col } from 'react-bootstrap';

const ENCODER_MODES = [
    { label: 'encoder-mode-none', value: 0 },
    { label: 'encoder-mode-left-analog-x', value: 1 },
    { label: 'encoder-mode-left-analog-y', value: 2 },
    { label: 'encoder-mode-right-analog-x', value: 3 },
    { label: 'encoder-mode-right-analog-y', value: 4 },
    { label: 'encoder-mode-left-trigger', value: 5 },
    { label: 'encoder-mode-right-trigger', value: 6 },
    { label: 'encoder-mode-dpad-x', value: 7 },
    { label: 'encoder-mode-dpad-y', value: 8 },
];

export const rotaryScheme = {
	RotaryAddonEnabled: yup
		.number()
		.required()
		.label('Rotary Encoder Add-On Enabled'),
    encoderOneEnabled: yup
        .boolean()
        .required()
		.label('Encoder One Enabled'),
    encoderOnePinA: yup
		.number()
		.label('Encoder One Pin A')
        .required()
		.validatePinWhenValue('RotaryAddonEnabled'),
    encoderOnePinB: yup
		.number()
		.label('Encoder One Pin B')
        .required()
		.validatePinWhenValue('RotaryAddonEnabled'),
    encoderOneMode: yup
		.number()
        .required()
		.label('Encoder One Mode'),
    encoderOnePPR: yup
		.number()
		.label('Encoder One PPR')
        .required(),
    encoderTwoEnabled: yup
        .boolean()
        .required()
		.label('Encoder Two Enabled'),
    encoderTwoPinA: yup
		.number()
		.label('Encoder Two Pin A')
        .required()
		.validatePinWhenValue('RotaryAddonEnabled'),
    encoderTwoPinB: yup
		.number()
		.label('Encoder Two Pin B')
        .required()
		.validatePinWhenValue('RotaryAddonEnabled'),
    encoderTwoMode: yup
		.number()
        .required()
		.label('Encoder Two Mode'),
    encoderTwoPPR: yup
		.number()
		.label('Encoder Two PPR')
        .required(),
};

export const rotaryState = {
    encoderOneEnabled: 0,
    encoderOnePinA: -1,
    encoderOnePinB: -1,
    encoderOneMode: 0,
    encoderOnePPR: 24,
    encoderTwoEnabled: 0,
    encoderTwoPinA: -1,
    encoderTwoPinB: -1,
    encoderTwoMode: 0,
    encoderTwoPPR: 24,
    RotaryAddonEnabled: 0,
};

const Rotary = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('Rotary:header-text')}>
			<div
				id="RotaryAddonEnabledOptions"
				hidden={!values.RotaryAddonEnabled}
			>
                <Row className="mb-3">
                    <div className="col-3">
                        <label>{t('Rotary:encoder-one-label')}</label>
                        <FormCheck
                            label={t('Common:switch-enabled')}
                            type="switch"
                            id="encoderOneEnabled"
                            isInvalid={false}
                            checked={Boolean(values.encoderOneEnabled)}
                            value={Boolean(values.encoderOneEnabled)}
                            onChange={(e) => {
                                handleCheckbox('encoderOneEnabled', values);
                                handleChange(e);
                            }}
                        />
                        <div id="" hidden={!values.encoderOneEnabled}>
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-pin-a-label')}
                                name="encoderOnePinA"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderOnePinA}
                                error={errors.encoderOnePinA}
                                isInvalid={errors.encoderOnePinA}
                                onChange={handleChange}
                                min={-1}
                                max={29}
                            />
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-pin-b-label')}
                                name="encoderOnePinB"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderOnePinB}
                                error={errors.encoderOnePinB}
                                isInvalid={errors.encoderOnePinB}
                                onChange={handleChange}
                                min={-1}
                                max={29}
                            />
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-ppr-label')}
                                name="encoderOnePPR"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderOnePPR}
                                error={errors.encoderOnePPR}
                                isInvalid={errors.encoderOnePPR}
                                onChange={handleChange}
                                min={24}
                                max={600}
                            />
                            <FormSelect
                                label={t('Rotary:encoder-mode-label')}
                                name="encoderOneMode"
                                className="form-select-sm"
                                groupClassName="mb-3"
                                value={values.encoderOneMode}
                                error={errors.encoderOneMode}
                                isInvalid={errors.encoderOneMode}
                                onChange={handleChange}
                            >
                                {ENCODER_MODES.map((o, i) => (
                                    <option
                                        key={`encoderOneMode-option-${i}`}
                                        value={o.value}
                                    >
                                        {t(`Rotary:${o.label}`)}
                                    </option>
                                ))}
                            </FormSelect>
                        </div>
                    </div>
                    <div className="col-3">
                        <label>{t('Rotary:encoder-two-label')}</label>
                        <FormCheck
                            label={t('Common:switch-enabled')}
                            type="switch"
                            id="encoderTwoEnabled"
                            isInvalid={false}
                            checked={Boolean(values.encoderTwoEnabled)}
                            value={Boolean(values.encoderTwoEnabled)}
                            onChange={(e) => {
                                handleCheckbox('encoderTwoEnabled', values);
                                handleChange(e);
                            }}
                        />
                        <div id="" hidden={!values.encoderTwoEnabled}>
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-pin-a-label')}
                                name="encoderTwoPinA"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderTwoPinA}
                                error={errors.encoderTwoPinA}
                                isInvalid={errors.encoderTwoPinA}
                                onChange={handleChange}
                                min={-1}
                                max={29}
                            />
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-pin-b-label')}
                                name="encoderTwoPinB"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderTwoPinB}
                                error={errors.encoderTwoPinB}
                                isInvalid={errors.encoderTwoPinB}
                                onChange={handleChange}
                                min={-1}
                                max={29}
                            />
                            <FormControl
                                type="number"
                                label={t('Rotary:encoder-ppr-label')}
                                name="encoderTwoPPR"
                                className="form-control-sm"
                                groupClassName="mb-3"
                                value={values.encoderTwoPPR}
                                error={errors.encoderTwoPPR}
                                isInvalid={errors.encoderTwoPPR}
                                onChange={handleChange}
                                min={24}
                                max={600}
                            />
                            <FormSelect
                                label={t('Rotary:encoder-mode-label')}
                                name="encoderTwoMode"
                                className="form-select-sm"
                                groupClassName="mb-3"
                                value={values.encoderTwoMode}
                                error={errors.encoderTwoMode}
                                isInvalid={errors.encoderTwoMode}
                                onChange={handleChange}
                            >
                                {ENCODER_MODES.map((o, i) => (
                                    <option
                                        key={`encoderTwoMode-option-${i}`}
                                        value={o.value}
                                    >
                                        {t(`Rotary:${o.label}`)}
                                    </option>
                                ))}
                            </FormSelect>
                        </div>
                    </div>
                </Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="RotaryAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.RotaryAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('RotaryAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Rotary;
