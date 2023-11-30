import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import GifCanvas from '../Components/GifCanvas';

export const gifScheme = {
    gifAddonEnabled: yup
        .number()
        .required()
        .label('Gifs Enabled'),
    gifAddonData: yup
        .array().of(yup.number())
        .required()
        .label('Gif data')
};

export const gifState = {
    gifAddonEnabled: 0,
    gifAddonData: Array(1036).fill(0),
};


const Gif = ({ values, errors, handleChange, handleCheckbox, setFieldValue }) => {
    const { t } = useTranslation()
    console.log(errors)
    return (
        <Section title={t('AddonsConfig:gif-header-text')}>
            <GifCanvas
                onChange={(arr: string) => {
                    setFieldValue('gifAddonData', arr)
                }}
                startingValue={values.gifAddonData}
            />
            <FormCheck
                label={t('Common:switch-enabled')}
                type="switch"
                id="GifButton"
                reverse
                isInvalid={false}
                checked={Boolean(values.gifAddonEnabled)}
                onChange={(e) => {
                    handleCheckbox('gifAddonEnabled', values);
                    handleChange(e);
                }}
            />
        </Section>
    );
};

export default Gif;
