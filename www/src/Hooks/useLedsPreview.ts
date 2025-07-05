import { useCallback } from 'react';
import WebApi from '../Services/WebApi';
import { useEffect } from 'react';
import { AnimationProfile } from '../Store/useLedStore';

const useLedsPreview = () => {
	// Turn off LEDs when the hook unmounts
	useEffect(() => turnOffLeds(), []);

	const activateLedsOnId = useCallback(
		async (pin: number, isCase: boolean = false) => {
			await WebApi.setAnimationButtonTestMode({
				TestData: {
					testMode: 2,
				},
			});
			await WebApi.setAnimationButtonTestState({
				TestLight: {
					testID: pin,
					testIsCaseLight: +isCase,
				},
			});
		},
		[],
	);

	const activateLedsProfile = useCallback(async (profile: AnimationProfile) => {
		WebApi.setAnimationButtonTestMode({
			TestData: {
				testMode: 4,
				testProfile: profile,
			},
		});
	}, []);

	const activateLedsChase = useCallback(async () => {
		WebApi.setAnimationButtonTestMode({
			TestData: {
				testMode: 3,
			},
		});
	}, []);

	const turnOffLeds = useCallback(() => {
		WebApi.setAnimationButtonTestMode({
			TestData: {
				testMode: 1,
			},
		});
	}, []);
	return {
		activateLedsOnId,
		activateLedsProfile,
		activateLedsChase,
		turnOffLeds,
	};
};

export default useLedsPreview;
