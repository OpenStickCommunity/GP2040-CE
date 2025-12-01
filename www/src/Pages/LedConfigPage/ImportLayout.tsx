import CustomSelect from '../../Components/CustomSelect';
import { Light } from '../../Store/useLedStore';
import WebApi from '../../Services/WebApi';
import { useEffect, useState } from 'react';
import isEqual from 'lodash/isEqual';

function ImportLayout({
	lights,
	setFieldValue,
}: {
	lights: Light[],
	setFieldValue: (field: string, value: any) => void;
}) {
	const [presets, setPresets] = useState<
		{ name: string; lightData: { Lights: Light[] } }[]
	>([]);

	useEffect(() => {
		WebApi.getLightsDataPresets().then(setPresets).catch(console.error);
	}, []);

	const handleImport = async (
		selectedOption: { value: string; label: string } | null,
	) => {
		if (selectedOption) {
			const preset = presets.find((p) => p.name === selectedOption.value);
			if (preset) {
				setFieldValue('Lights', preset.lightData.Lights);
			}
		}
	};

	const options = presets.map((preset) => ({
		value: preset.name,
		label: preset.name,
	}));

	const selectedPreset = presets.find(preset => isEqual(preset.lightData.Lights, lights));

	return (
		<>
			<hr className="d-md-none" />
			<p>Choose from predefined installed layouts</p>
			<CustomSelect
				placeholder="Select Layout..."
				options={options}
				value={selectedPreset?.name ? {
					value: selectedPreset!.name,
					label: selectedPreset!.name,
				} : null	}
				isMulti={false}
				onChange={handleImport}
			/>
		</>
	);
}

export default ImportLayout;
