import CustomSelect from '../../Components/CustomSelect';
import useLedStore from '../../Store/useLedStore';
import WebApi from '../../Services/WebApi';

function ImportLayout({
	setFieldValue,
}: {
	setFieldValue: (field: string, value: any) => void;
}) {
	const boardDefaultSetupNames = useLedStore(
		(state) => state.boardDefaultSetupNames,
	);
	const handleImport = async (
		selectedOption: { value: string; label: string } | null,
	) => {
		if (selectedOption) {
			const { LightData } = await WebApi.getLightsDataFromPreset(
				selectedOption.value,
			);
			setFieldValue('Lights', LightData.Lights);
		}
	};

	return (
		<>
			<hr className="d-md-none" />
			<p>Choose from predefined installed layouts</p>
			<CustomSelect
				placeholder="Select Layout..."
				options={boardDefaultSetupNames.map((name) => ({
					value: name,
					label: name,
				}))}
				isMulti={false}
				onChange={handleImport}
			/>
		</>
	);
}

export default ImportLayout;
