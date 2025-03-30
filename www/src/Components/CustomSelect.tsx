import ReactSelect, { GroupBase, Props } from 'react-select';

import './CustomSelect.scss';

function CustomSelect<
	Option,
	IsMulti extends boolean = false,
	Group extends GroupBase<Option> = GroupBase<Option>,
>(props: Props<Option, IsMulti, Group>) {
	return (
		<ReactSelect
			className="react-select__container"
			classNamePrefix="react-select"
			{...props}
		/>
	);
}

export default CustomSelect;
