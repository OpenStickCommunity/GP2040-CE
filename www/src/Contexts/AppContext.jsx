import React, { createContext, useEffect, useState } from 'react';
import * as yup from 'yup';

import WebApi from '../Services/WebApi';

export const AppContext = createContext(null);

let checkPins = null;

yup.addMethod(yup.string, 'validateColor', function() {
	return this.test('', 'Valid hex color required', (value) => value?.match(/^#([0-9a-f]{3}|[0-9a-f]{6})$/i));
});

yup.addMethod(yup.NumberSchema, 'validateSelectionWhenValue', function(name, choices) {
	return this.when(name, {
		is: value => !isNaN(parseInt(value)),
		then: () => this.required().oneOf(choices.map(o => o.value)),
		otherwise: () => yup.mixed().notRequired()
	})
});

yup.addMethod(yup.NumberSchema, 'validateNumberWhenValue', function(name) {
	return this.when(name, {
		is: value => !isNaN(parseInt(value)),
		then: () => this.required(),
		otherwise: () => yup.mixed().notRequired().strip()
	})
});

yup.addMethod(yup.NumberSchema, 'validateMinWhenEqualTo', function(name, compareValue, min) {
	return this.when(name, {
		is: value => parseInt(value) === compareValue,
		then: () => this.required().min(min),
		otherwise: () => yup.mixed().notRequired().strip()
	})
});

yup.addMethod(yup.NumberSchema, 'validateRangeWhenValue', function(name, min, max) {
	return this.when(name, {
		is: value => !isNaN(parseInt(value)),
		then: () => this.required().min(min).max(max),
		otherwise: () => yup.mixed().notRequired().strip()
	});
});

yup.addMethod(yup.NumberSchema, 'validatePinWhenEqualTo', function(name, compareName, compareValue) {
	return this.when(compareName, {
		is: value => parseInt(value) === compareValue,
		then: () => this.validatePinWhenValue(name),
		otherwise: () => yup.mixed().notRequired().strip()
	})
});

yup.addMethod(yup.NumberSchema, 'validatePinWhenValue', function(name) {
	return this.checkUsedPins();
});

yup.addMethod(yup.NumberSchema, 'checkUsedPins', function() {
	return this.test('', '${originalValue} is unavailable/already assigned!', (value) => checkPins(value));
});

export const AppContextProvider = ({ children, ...props }) => {
	const localValue = localStorage.getItem('buttonLabelType') || 'gp2040';
	const localValue2 = localStorage.getItem('swapTpShareLabels') || false;
	const [buttonLabels, _setButtonLabels] = useState({ swapTpShareLabels: localValue2, buttonLabelType: localValue });
	const setButtonLabels = ({ buttonLabelType : newType, swapTpShareLabels: newSwap }) => {
		console.log('buttonLabelType is', newType)
		newType && localStorage.setItem('buttonLabelType', newType);
		newSwap !== undefined && localStorage.setItem('swapTpShareLabels', newSwap);
		_setButtonLabels(({ buttonLabelType, swapTpShareLabels }) =>
			({ buttonLabelType: newType || buttonLabelType,
			   swapTpShareLabels: (newSwap !== undefined) ? newSwap : swapTpShareLabels }));
	};

	const [savedColors, _setSavedColors] = useState(localStorage.getItem('savedColors') ? localStorage.getItem('savedColors').split(',') : []);
	const setSavedColors = (savedColors) => {
		localStorage.setItem('savedColors', savedColors);
		_setSavedColors(savedColors);
	};

	const updateButtonLabels = (e) => {
		const { key, newValue } = e;
		if (key === "swapTpShareLabels") {
		  _setButtonLabels(({ buttonLabelType }) => ({ buttonLabelType, swapTpShareLabels: newValue === "true" }));
		}
		if (key === "buttonLabelType") {
		  _setButtonLabels(({ swapTpShareLabels }) => ({ buttonLabelType: newValue, swapTpShareLabels }));
		}
	};

	useEffect(() => {
		_setButtonLabels({ buttonLabelType: buttonLabels.buttonLabelType,
						   swapTpShareLabels: buttonLabels.swapTpShareLabels });
		window.addEventListener("storage", updateButtonLabels);
		return () => {
		  window.removeEventListener("storage", updateButtonLabels);
		};
	  }, []);

	const [gradientNormalColor1, _setGradientNormalColor1] = useState('#00ffff');
	const setGradientNormalColor1 = (gradientNormalColor1) => {
		localStorage.setItem('gradientNormalColor1', gradientNormalColor1);
		_setGradientNormalColor1(gradientNormalColor1);
	};

	const [gradientNormalColor2, _setGradientNormalColor2] = useState('#ff00ff');
	const setGradientNormalColor2 = (gradientNormalColor2) => {
		localStorage.setItem('gradientNormalColor2', gradientNormalColor2);
		_setGradientNormalColor1(gradientNormalColor2);
	};

	const [gradientPressedColor1, _setGradientPressedColor1] = useState('#ff00ff');
	const setGradientPressedColor1 = (gradientPressedColor1) => {
		localStorage.setItem('gradientPressedColor1', gradientPressedColor1);
		_setGradientPressedColor1(gradientPressedColor1);
	};

	const [gradientPressedColor2, _setGradientPressedColor2] = useState('#00ffff');
	const setGradientPressedColor2 = (gradientPressedColor2) => {
		localStorage.setItem('gradientPressedColor2', gradientPressedColor2);
		_setGradientPressedColor1(gradientPressedColor2);
	};

	const [usedPins, setUsedPins] = useState([]);

	const updateUsedPins = async () => {
		const data = await WebApi.getUsedPins(setLoading);
		setUsedPins(data.usedPins);
		console.log('usedPins updated:', data.usedPins);
		return data;
	};

	useEffect(() => {
		updateUsedPins();
	}, []);

	useEffect(() => {
		checkPins = (value) => {
			const hasValue = value > -1;
			const isValid = value === undefined
				|| value === -1
				|| (hasValue && value < 30 && (usedPins || []).indexOf(value) === -1);
			return isValid;
		};
	}, [usedPins, setUsedPins]);

	console.log('usedPins:', usedPins);

	const [savedColorScheme, _setSavedColorScheme] = useState(localStorage.getItem('savedColorScheme') || 'auto');
	const setSavedColorScheme = (savedColorScheme) => {
		localStorage.setItem('savedColorScheme', savedColorScheme);
		_setSavedColorScheme(savedColorScheme);
	};

	const [savedLanguage, _setSavedLanguage] = useState(localStorage.getItem('i18next') || 'en-GB');
	const setSavedLanguage = (savedLanguage) => {
		localStorage.setItem('i18next', savedLanguage);
		_setSavedLanguage(savedLanguage);
	};

	const [loading, setLoading] = useState(false);

	return (
		<AppContext.Provider
			{...props}
			value={{
				buttonLabels,
				gradientNormalColor1,
				gradientNormalColor2,
				gradientPressedColor1,
				gradientPressedColor2,
				savedColors,
				usedPins,
				setButtonLabels,
				setGradientNormalColor1,
				setGradientNormalColor2,
				setGradientPressedColor1,
				setGradientPressedColor2,
				setSavedColors,
				setUsedPins,
				updateUsedPins,
				savedColorScheme,
				setSavedColorScheme,
				savedLanguage,
				setSavedLanguage,
				loading,
				setLoading
			}}
		>
			{children}
		</AppContext.Provider>
	);
};
