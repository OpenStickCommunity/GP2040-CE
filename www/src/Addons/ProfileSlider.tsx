import { useTranslation, Trans } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const profileSliderScheme = {
	SliderProfileInputEnabled: yup
		.number()
		.required()
		.label('Slider Profile Input Enabled'),
	sliderProfileNumPositions: yup
		.number()
		.label('Slider Profile Num Positions')
		.min(2)
		.max(8)
		.typeError('Slider Profile Num Positions must be a number'),
	sliderProfileDefaultProfile: yup
		.number()
		.label('Slider Profile Default Profile')
		.min(1)
		.max(6),
	sliderProfileAssignments: yup
		.array()
		.of(yup.number())
		.label('Slider Profile Assignments'),
};

export const profileSliderState = {
	SliderProfileInputEnabled: 0,
	sliderProfileNumPositions: 2,
	sliderProfileDefaultProfile: 1,
	sliderProfileAssignments: [1, 2, 3, 4, 5, 6, 7, 8],
};

const ProfileSlider = ({ values, errors, handleChange, handleCheckbox, setFieldValue }: AddonPropTypes) => {
	const { t } = useTranslation();
	const numPositions = (values as any).sliderProfileNumPositions || 2;

	// Generate profile options (1 through 6)
	const profileOptions = Array.from({ length: 6 }, (_, i) => ({
		label: t('PinMapping:profile-label-default', { profileNumber: i + 1 }),
		value: i + 1,
	}));

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/profile-selection-slider"
					target="_blank"
					className="text-reset text-decoration-none"
				>
					{t('AddonsConfig:profile-slider-header-text')}
				</a>
			}
		>
			<div id="SliderProfileInputOptions" hidden={!(values as any).SliderProfileInputEnabled}>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:profile-slider-sub-header-text')}
				</div>
				<div className="alert alert-info" role="alert">
					<Trans
						ns="AddonsConfig"
						i18nKey="AddonsConfig:pin-config-moved-to-core-text"
						components={[
							<a
								key="0"
								href="../pin-mapping"
								className="alert-link"
								target="_blank"
							/>
						]}
					/>
				</div>

				<Row className="mb-3">
					<FormControl
						label={t('AddonsConfig:profile-slider-num-positions-label')}
						name="sliderProfileNumPositions"
						type="number"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={numPositions}
						error={(errors as any).sliderProfileNumPositions}
						isInvalid={Boolean((errors as any).sliderProfileNumPositions)}
						onChange={(e: any) => {
							const newVal = parseInt(e.target.value, 10);
							setFieldValue('sliderProfileNumPositions', newVal);
							handleChange(e);
						}}
						min={2}
						max={8}
					/>
				</Row>

				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:profile-slider-default-profile-label')}
						name="sliderProfileDefaultProfile"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={(values as any).sliderProfileDefaultProfile || 1}
						error={(errors as any).sliderProfileDefaultProfile}
						isInvalid={Boolean((errors as any).sliderProfileDefaultProfile)}
						onChange={(e: any) => {
							setFieldValue('sliderProfileDefaultProfile', parseInt(e.target.value, 10));
							handleChange(e);
						}}
					>
						{profileOptions.map((o) => (
							<option key={`default-profile-option-${o.value}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>

				{Array.from({ length: numPositions }, (_, i) => (
					<Row key={`profile-position-${i}`} className="mb-3">
						<FormSelect
							label={t('AddonsConfig:profile-slider-position-label', { position: i + 1 })}
							name={`sliderProfileAssignments[${i}]`}
							className="form-select-sm"
							groupClassName="col-sm-3 mb-3"
							value={
								(values as any).sliderProfileAssignments && (values as any).sliderProfileAssignments[i]
									? (values as any).sliderProfileAssignments[i]
									: i + 1
							}
							error={(errors as any).sliderProfileAssignments ? (errors as any).sliderProfileAssignments[i] : undefined}
							isInvalid={Boolean((errors as any).sliderProfileAssignments && (errors as any).sliderProfileAssignments[i])}
							onChange={(e: any) => {
								const newAssignments = [...((values as any).sliderProfileAssignments || [])];
								newAssignments[i] = parseInt(e.target.value, 10);
								setFieldValue('sliderProfileAssignments', newAssignments);
							}}
						>
							{profileOptions.map((o) => (
								<option key={`profile-option-${o.value}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					</Row>
				))}
			</div>

			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="SliderProfileInputButton"
				reverse
				isInvalid={false}
				checked={Boolean((values as any).SliderProfileInputEnabled)}
				onChange={(e: any) => {
					handleCheckbox('SliderProfileInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ProfileSlider;
