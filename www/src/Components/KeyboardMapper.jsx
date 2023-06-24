import React from 'react';
import { useTranslation } from 'react-i18next';
import FormSelect from './FormSelect';
import { KEY_CODES } from "../Data/Keyboard";
import { BUTTONS } from '../Data/Buttons';
import boards from '../Data/Boards.json';

const KeyboardMapper = ({ buttonLabels, handleKeyChange, validated, getKeyMappingForButton, ...props }) => {
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const { t } = useTranslation('Components', { keyPrefix: 'keyboard-mapper' });

	return (
		<table className="table table-sm pin-mapping-table" {...props}>
			<thead className="table">
				<tr>
					<th className="table-header-button-label">{BUTTONS[buttonLabelType].label}</th>
					<th>{t('key-header')}</th>
				</tr>
			</thead>
			<tbody>
				{Object.keys(BUTTONS[buttonLabelType])?.filter(p => p !== 'label' && p !== 'value').map((button, i) => {
					let label = BUTTONS[buttonLabelType][button];
					if (button === "S1" && swapTpShareLabels) {
						label = BUTTONS[buttonLabelType]["A2"];
					}
					if (button === "A2" && swapTpShareLabels) {
						label = BUTTONS[buttonLabelType]["S1"];
					}
					const keyMapping = getKeyMappingForButton(button);
					return <tr key={`button-map-${i}`} className={validated && !!keyMapping.error ? "table-danger" : ""}>
						<td>{label}</td>
						<td>
							<FormSelect
								type="number"
								className="form-select-sm sm-3"
								value={keyMapping.key}
								isInvalid={!!keyMapping.error}
								error={keyMapping.error}
								onChange={(e) => handleKeyChange(e.target.value ? parseInt(e.target.value) : '', button)}
							>
								{KEY_CODES.map((o, i) => <option key={`button-key-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</td>
					</tr>
				})}
			</tbody>
		</table>
	);
};

export const validateMappings = (mappings, t) => {
	const props = Object.keys(mappings);

	for (let prop of props) {
		mappings[prop].error = null;

		for (let otherProp of props) {
			if (prop === otherProp)
				continue;

			const key = KEY_CODES.find(({ _, value }) => mappings[prop].key === value).label;

			if (mappings[prop].key !== 0x00 && mappings[prop].key === mappings[otherProp].key) {
				mappings[prop].error = t('Components:keyboard-mapper.error-conflict', { key });
			} else if ((boards[import.meta.env.VITE_GP2040_BOARD].invalidKeys || []).filter(p => p === mappings[prop].key).length > 0) {
				mappings[prop].error = t('Components:keyboard-mapper.error-invalid', { key });
			}
		}

	}

	return mappings;
};

export default KeyboardMapper;
