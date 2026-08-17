import { FormikErrors } from 'formik';

import {
	AnimationOptions,
	AnimationProfile,
	LedOptions,
	Light,
} from '../../Store/useLedStore';

export type LedFormValues = {
	ledOptions: LedOptions;
	Lights: Light[];
	AnimationOptions: AnimationOptions;
};

export const getLightError = (
	errors: FormikErrors<LedFormValues>,
	index: number,
): FormikErrors<Light> | undefined =>
	errors.Lights?.[index] as FormikErrors<Light> | undefined;

export const getProfileError = (
	errors: FormikErrors<LedFormValues>,
	index: number,
): FormikErrors<AnimationProfile> | undefined =>
	errors.AnimationOptions?.profiles?.[index] as
		| FormikErrors<AnimationProfile>
		| undefined;
