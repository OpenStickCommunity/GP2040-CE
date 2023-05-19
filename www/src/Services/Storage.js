const STORAGE_BUTTON_LABELS = 'buttonLabels';

const loadButtonLabels = () => localStorage.getItem(STORAGE_BUTTON_LABELS) ?? 'gp2040';
const saveButtonLabels = (value) => localStorage.setItem(STORAGE_BUTTON_LABELS, value);

export {
	loadButtonLabels,
	saveButtonLabels,
};
