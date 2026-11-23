type JoyConControllerProps = {
	side: 'left' | 'right';
	body: string;
	buttons: string;
	active?: 'body' | 'buttons' | null;
};

const JoyConController = ({
	side,
	body,
	buttons,
	active = null,
}: JoyConControllerProps) => {
	const part = (name: JoyConControllerProps['active']) => ({
		opacity: active && active !== name ? 0.2 : 1,
		style: { transition: 'opacity 150ms' },
	});
	return (
		<svg
			xmlns="http://www.w3.org/2000/svg"
			viewBox="0 0 320 120"
			fillRule="evenodd"
			width="100%"
			role="img"
		>
			{side === 'left' ? (
				<>
					<g fill={body} {...part('body')}>
						<path d="M3.5,52L3.5,12L320,12L320,52C320,87.4,291.3,116,255.9,116C255.9,116,67.5,116.2,67.5,116.2C32.2,116.2,3.5,87.4,3.5,52Z" />
						<path d="M73.7,5.5L73.7,2.8L100.7,2.8L100.7,5.5L73.7,5.5Z" />
						<rect x="202.3" y="2.8" width="27" height="2.7" />
					</g>
					<g fill="#000000">
						<path d="M20.8,12L20.8,3.2L67,3.2L73,5.5L102,5.5L108,3.2L196,3.2L202,5.5L230,5.5L236,3.2L287.7,3.2L287.7,8.2L274.6,8.2L274.6,12L20.8,12Z" />
					</g>
					<g fill={buttons} {...part('buttons')}>
						<path d="M43.9,111.7L42.1,115.2C17.6,106,0,81.3,0,52.2L0,30.8L3.5,30.8L3.5,52C3.5,79.1,20.3,102.3,43.9,111.7Z" />
						<rect x="220.3" y="33" width="20" height="20" />
						<rect x="31.5" y="19.7" width="6" height="18" />
						<circle cx="194.9" cy="60" r="12.5" />
						<circle cx="171.1" cy="83.9" r="12.5" />
						<circle cx="147.2" cy="60" r="12.5" />
						<circle cx="82.7" cy="60" r="25" />
						<circle cx="171.1" cy="36.1" r="12.5" />
					</g>
				</>
			) : (
				<>
					<g fill={body} {...part('body')}>
						<path d="M316.5,52L316.5,12L0,12L0,52C0,87.4,28.7,116,64.1,116C64.1,116,252.5,116.2,252.5,116.2C287.8,116.2,316.5,87.4,316.5,52Z" />
						<path d="M246.3,5.5L246.3,2.8L219.3,2.8L219.3,5.5L246.3,5.5Z" />
						<rect x="90.7" y="2.8" width="27" height="2.7" />
					</g>
					<g fill="#000000">
						<path d="M299.2,12L299.2,3.2L253,3.2L247,5.5L218,5.5L212,3.2L124,3.2L118,5.5L90,5.5L84,3.2L32.3,3.2L32.3,8.2L45.4,8.2L45.4,12L299.2,12Z" />
					</g>
					<g fill={buttons} {...part('buttons')}>
						<path d="M276.1,111.7L277.9,115.2C302.4,106,320,81.3,320,52.2L320,30.8L316.5,30.8L316.5,52C316.5,79.1,299.7,102.3,276.1,111.7Z" />
						<path d="M282.5,25.7L282.5,19.7L288.5,19.7L288.5,25.7L294.5,25.7L294.5,31.7L288.5,31.7L288.5,37.7L282.5,37.7L282.5,31.7L276.5,31.7L276.5,25.7L282.5,25.7Z" />
						<circle cx="212.3" cy="60" r="12.5" />
						<circle cx="89.8" cy="43.6" r="12.5" />
						<circle cx="236.2" cy="83.9" r="12.5" />
						<circle cx="260.1" cy="60" r="12.5" />
						<circle cx="149.5" cy="61.1" r="25" />
						<circle cx="236.2" cy="36.1" r="12.5" />
					</g>
				</>
			)}
		</svg>
	);
};
export default JoyConController;
