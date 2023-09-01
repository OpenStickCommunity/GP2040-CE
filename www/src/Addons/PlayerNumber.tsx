import React from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

export const playerNumberScheme = {
	PlayerNumAddonEnabled: yup
		.number()
		.required()
		.label('Player Number Add-On Enabled'),
	playerNumber: yup
		.number()
		.label('Player Number')
		.validateRangeWhenValue('PlayerNumAddonEnabled', 1, 4),
};

export const playerNumberState = {
	playerNumber: 1,
	PlayerNumAddonEnabled: 0,
};

const PlayerNumber = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:player-number-header-text')}>
			<div id="PlayerNumAddonOptions" hidden={!values.PlayerNumAddonEnabled}>
				<Alert variant="danger">
					<Alert.Heading>
						{t('AddonsConfig:player-number-sub-header')}
					</Alert.Heading>
					<p>{t('AddonsConfig:player-number-sub-header-text')}</p>
				</Alert>

				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:player-number-label')}
						name="playerNumber"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.playerNumber}
						error={errors.playerNumber}
						isInvalid={errors.playerNumber}
						onChange={handleChange}
						min={1}
						max={4}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="PlayerNumAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.PlayerNumAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('PlayerNumAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default PlayerNumber;
