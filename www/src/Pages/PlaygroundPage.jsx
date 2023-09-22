import React, { useEffect } from 'react';
import Select from 'react-select';
import { Button } from 'react-bootstrap';
import invert from 'lodash/invert';
import map from 'lodash/map';

import usePinStore, {
	BUTTON_ACTIONS,
	NON_SELECTABLE_BUTTON_ACTIONS,
} from '../Store/usePinStore';

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([_, value]) => !isNonSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

const getOption = (actionId) => ({
	label: invert(BUTTON_ACTIONS)[actionId],
	value: actionId,
});

export default function PlaygroundPage() {
	const { pins, setPinAction, fetchPins, savePins } = usePinStore();

	useEffect(() => {
		fetchPins();
	}, []);

	return (
		<div>
			<h2>Button mapping</h2>
			<div style={{ columnCount: 2 }}>
				{map(pins, (pinAction, pin) => (
					<div
						key={`pin-${pin}`}
						style={{
							padding: '20px 0',
							borderTop: '1px solid black',
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<label style={{ alignSelf: 'center', paddingRight: '1em' }}>
							{pin.toUpperCase()}
						</label>
						<div style={{ flexGrow: 1, color: 'black' }}>
							<Select
								isClearable
								isSearchable
								options={options}
								value={getOption(pinAction)}
								isDisabled={isNonSelectable(pinAction)}
								onChange={(change) =>
									setPinAction(
										pin,
										change?.value === undefined ? -10 : change.value, // On clear set to -10
									)
								}
							/>
						</div>
					</div>
				))}
			</div>
			<Button variant="primary" onClick={() => savePins()}>
				{'Save'}
			</Button>
		</div>
	);
}
