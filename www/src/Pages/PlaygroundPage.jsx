import React, { useEffect } from 'react';
import Select from 'react-select';
import { Button, Form } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import map from 'lodash/map';

import usePinStore, {
	BUTTON_ACTIONS,
	NON_SELECTABLE_BUTTON_ACTIONS,
} from '../Store/usePinStore';

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const createOptions = (t) =>
	Object.entries(BUTTON_ACTIONS)
		.filter(([_, value]) => !isNonSelectable(value))
		.map(([key, value]) => ({
			label: t(`PinMapping:actions.${key}`),
			value,
		}));

const getOption = (t, actionId) => ({
	label: t(`PinMapping:actions.${invert(BUTTON_ACTIONS)[actionId]}`),
	value: actionId,
});

export default function PlaygroundPage() {
	const { pins, setPinAction, fetchPins, savePins } = usePinStore();
	const { t } = useTranslation('');
	const options = createOptions(t);
	useEffect(() => {
		fetchPins();
	}, []);

	return (
		<Form onSubmit={() => savePins()}>
			<h1>Button mapping</h1>
			<div className="row row-cols-lg-3 row-cols-md-2 gx-3">
				{map(pins, (pinAction, pin) => (
					<div
						key={`pin-${pin}`}
						className="d-flex justify-content-center py-2"
					>
						<div className="d-flex align-items-center pe-2">
							<label htmlFor={pin}>{pin.toUpperCase()}</label>
						</div>
						<Select
							id={pin}
							className="flex-grow-1 text-primary"
							isClearable
							isSearchable
							options={options}
							value={getOption(t, pinAction)}
							isDisabled={isNonSelectable(pinAction)}
							onChange={(change) =>
								setPinAction(
									pin,
									change?.value === undefined ? -10 : change.value, // On clear set to -10
								)
							}
						/>
					</div>
				))}
			</div>

			<Button type="submit">{t('Common:button-save-label')}</Button>
		</Form>
	);
}
