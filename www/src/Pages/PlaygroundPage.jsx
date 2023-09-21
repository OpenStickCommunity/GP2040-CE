import React, { useEffect } from 'react';
import Select from 'react-select';
import usePinStore, { BUTTON_ACTIONS } from '../Store/usePinStore';
import invert from 'lodash/invert';

const options = Object.entries(BUTTON_ACTIONS).map(([key, value]) => ({
	label: key,
	value,
}));

const getOption = (actionId) => ({
	label: invert(BUTTON_ACTIONS)[actionId],
	value: actionId,
});

export default function PlaygroundPage() {
	const { pins, setPinAction, fetchPins } = usePinStore();

	useEffect(() => {
		fetchPins();
	}, []);

	return (
		<div>
			<h2>Button mapping</h2>
			<div style={{ columnCount: 2 }}>
				{Object.values(pins).map((pin, pinId) => (
					<div
						key={pinId}
						style={{
							padding: '20px 0',
							borderTop: '1px solid black',
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<label
							style={{ alignSelf: 'center', paddingRight: '1em' }}
						>{`GP${pinId}`}</label>
						<div style={{ flexGrow: 1, color: 'black' }}>
							<Select
								isClearable
								isSearchable
								options={options}
								value={getOption(pin.action)}
								onChange={(change) => setPinAction(pinId, change?.value)}
							/>
						</div>
					</div>
				))}
			</div>
		</div>
	);
}
