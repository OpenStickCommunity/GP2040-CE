import ReactSelect, { GroupBase, Props } from 'react-select';

import './CustomSelect.scss';

function CustomSelect<
	Option,
	IsMulti extends boolean = false,
	Group extends GroupBase<Option> = GroupBase<Option>,
>(props: Props<Option, IsMulti, Group> & { isInvalid?: boolean }) {
	return (
		<ReactSelect
			className="react-select__container"
			classNamePrefix="react-select"
			classNames={{
				control: (_) =>
					props['isInvalid']
						? 'react-select__control--is-invalid'
						: 'react-select__control',
			}}
			{...props}
		/>
	);
}

export default CustomSelect;
