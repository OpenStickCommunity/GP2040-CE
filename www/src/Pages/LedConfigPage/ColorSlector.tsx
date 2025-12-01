import { StylesConfig } from 'react-select';
import CustomSelect from '../../Components/CustomSelect';
import LED_COLORS from '../../Data/Leds';

type ColorOption = {
	value: number;
	label: string;
	color: string;
};

const colorDot = (color = 'transparent') => ({
	alignItems: 'center',
	display: 'flex',

	':before': {
		backgroundColor: color,
		borderRadius: 15,
		content: '" "',
		display: 'block',
		flexShrink: 0,
		marginRight: 8,
		height: 15,
		width: 15,
	},
});

const colorStyles: StylesConfig<(typeof LED_COLORS)[number]> = {
	control: (styles) => ({ ...styles, backgroundColor: 'white' }),
	option: (styles, { data }) => ({ ...styles, ...colorDot(data.color) }),
	input: (styles) => ({ ...styles }),
	placeholder: (styles) => ({ ...styles, ...colorDot('#ccc') }),
	singleValue: (styles, { data }) => ({
		...styles,
		...colorDot(data.color),
	}),
};

function ColorSelector({
	options,
	value,
	onChange,
}: {
	options: ColorOption[];
	value?: ColorOption | null;
	onChange: (selected: ColorOption | null) => void;
}) {
	return (
		<CustomSelect
			options={options}
			styles={colorStyles}
			isMulti={false}
			onChange={onChange}
			value={value}
		/>
	);
}

export default ColorSelector;
