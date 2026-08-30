import { ChangeEvent } from 'react';
import { useTranslation, Trans } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

// matches MAX_PROFILE_SLIDER_POSITIONS and the proto max_count
const MAX_SLIDER_POSITIONS = 8;
const MIN_SLIDER_POSITIONS = 2;
// core profile + 5 alternate gpioMappingsSets
const MAX_PROFILES = 6;

export const profileSliderScheme = {
	SliderProfileInputEnabled: yup
		.number()
		.required()
		.label('Slider Profile Input Enabled'),
	sliderProfileNumPositions: yup
		.number()
		.label('Slider Profile Num Positions')
		.min(MIN_SLIDER_POSITIONS)
		.max(MAX_SLIDER_POSITIONS)
		.typeError('Slider Profile Num Positions must be a number'),
	sliderProfileDefaultProfile: yup
		.number()
		.label('Slider Profile Default Profile')
		.min(1)
		.max(MAX_PROFILES),
	sliderProfileAssignments: yup
		.array()
		.of(yup.number().min(1).max(MAX_PROFILES))
		.label('Slider Profile Assignments'),
};

export const profileSliderState = {
	SliderProfileInputEnabled: 0,
	sliderProfileNumPositions: 2,
	sliderProfileDefaultProfile: 1,
	sliderProfileAssignments: [1, 2, 3, 4, 5, 6, 7, 8],
};

const ProfileSlider = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}: AddonPropTypes) => {
	const { t } = useTranslation();
	const numPositions = Math.min(
		Math.max(
			Number(values.sliderProfileNumPositions) || MIN_SLIDER_POSITIONS,
			MIN_SLIDER_POSITIONS,
		),
		MAX_SLIDER_POSITIONS,
	);
	const assignmentErrors = Array.isArray(errors.sliderProfileAssignments)
		? errors.sliderProfileAssignments
		: [];

	// position i falls back to profile i + 1 when it has no assignment yet
	const assignmentForPosition = (i: number) =>
		values.sliderProfileAssignments?.[i] || Math.min(i + 1, MAX_PROFILES);

	const profileOptions = Array.from({ length: MAX_PROFILES }, (_, i) => ({
		label: t('PinMapping:profile-label-default', { profileNumber: i + 1 }),
		value: i + 1,
	}));

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/profile-selection-slider"
					target="_blank"
					rel="noreferrer"
					className="text-reset text-decoration-none"
				>
					{t('AddonsConfig:profile-slider-header-text')}
				</a>
			}
		>
			<div
				id="SliderProfileInputOptions"
				hidden={!values.SliderProfileInputEnabled}
			>
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
							/>,
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
						value={values.sliderProfileNumPositions}
						error={errors.sliderProfileNumPositions}
						isInvalid={Boolean(errors.sliderProfileNumPositions)}
						onChange={handleChange}
						min={MIN_SLIDER_POSITIONS}
						max={MAX_SLIDER_POSITIONS}
					/>
				</Row>

				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:profile-slider-default-profile-label')}
						name="sliderProfileDefaultProfile"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderProfileDefaultProfile || 1}
						error={errors.sliderProfileDefaultProfile}
						isInvalid={Boolean(errors.sliderProfileDefaultProfile)}
						onChange={handleChange}
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
							label={t('AddonsConfig:profile-slider-position-label', {
								position: i + 1,
							})}
							name={`sliderProfileAssignments[${i}]`}
							className="form-select-sm"
							groupClassName="col-sm-3 mb-3"
							value={assignmentForPosition(i)}
							error={assignmentErrors[i]}
							isInvalid={Boolean(assignmentErrors[i])}
							onChange={(e: ChangeEvent<HTMLSelectElement>) => {
								// fill every position so the array has no holes, which
								// the firmware would otherwise read as profile 0
								const newAssignments = Array.from(
									{ length: MAX_SLIDER_POSITIONS },
									(_, idx) => assignmentForPosition(idx),
								);
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
				checked={Boolean(values.SliderProfileInputEnabled)}
				onChange={(e) => {
					handleCheckbox('SliderProfileInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ProfileSlider;
