import CustomSelect from '../../Components/CustomSelect';
import { Light } from '../../Store/useLedStore';
import WebApi from '../../Services/WebApi';
import { useEffect, useState } from 'react';

function ImportLayout({
	setFieldValue,
}: {
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

	return (
		<>
			<hr className="d-md-none" />
			<p>Choose from predefined installed layouts</p>
			<CustomSelect
				placeholder="Select Layout..."
				options={presets.map((preset) => ({
					value: preset.name,
					label: preset.name,
				}))}
				isMulti={false}
				onChange={handleImport}
			/>
		</>
	);
}

export default ImportLayout;
