type JoyConPairControllerProps = {
	leftBody: string;
	leftButtons: string;
	rightBody: string;
	rightButtons: string;
	active?: 'leftBody' | 'leftButtons' | 'rightBody' | 'rightButtons' | null;
};

const JoyConPairController = ({
	leftBody,
	leftButtons,
	rightBody,
	rightButtons,
	active = null,
}: JoyConPairControllerProps) => {
	const part = (name: JoyConPairControllerProps['active']) => ({
		opacity: active && active !== name ? 0.2 : 1,
		style: { transition: 'opacity 150ms' },
	});
	return (
		<svg
			xmlns="http://www.w3.org/2000/svg"
			viewBox="0 0 260 320"
			fillRule="evenodd"
			width="100%"
			role="img"
		>
			<g fill={leftBody} {...part('leftBody')}>
				<path d="M69.1,3.5L109.2,3.5L109.2,320L69.1,320C33.8,320,5.1,291.3,5.1,255.9C5.1,255.9,5,67.5,5,67.5C5,32.2,33.7,3.5,69.1,3.5Z" />
				<path d="M115.7,73.7L118.4,73.7L118.4,100.7L115.6,100.7L115.7,73.7Z" />
				<rect x="115.7" y="202.3" width="2.7" height="27" />
			</g>
			<g fill={rightBody} {...part('rightBody')}>
				<path d="M190.9,3.5L150.8,3.5L150.8,320L190.9,320C226.2,320,254.9,291.3,254.9,255.9C254.9,255.9,255,67.5,255,67.5C255,32.2,226.3,3.5,190.9,3.5Z" />
				<path d="M144.3,73.7L141.6,73.7L141.6,100.7L144.4,100.7L144.3,73.7Z" />
				<rect x="141.6" y="202.3" width="2.7" height="27" />
			</g>
			<g fill="#000000">
				<path d="M109.2,20.8L118,20.8L118,67L115.7,73L115.6,102L118,108L118,196L115.7,202L115.7,230L118,236L118,287.7L113,287.7L113,274.6L109.2,274.6L109.2,20.8Z" />
				<path d="M150.8,20.8L142,20.8L142,67L144.3,73L144.4,102L142,108L142,196L144.3,202L144.3,230L142,236L142,287.7L147,287.7L147,274.6L150.8,274.6L150.8,20.8Z" />
			</g>
			<g fill={leftButtons} {...part('leftButtons')}>
				<path d="M9.5,43.9L6,42.1C15.2,17.6,39.9,0,68.9,0L90.4,0L90.3,3.5L69.1,3.5C42.1,3.5,18.9,20.3,9.5,43.9Z" />
				<rect x="68.2" y="220.3" width="20" height="20" />
				<rect x="83.5" y="31.5" width="18" height="6" />
				<circle cx="61.2" cy="194.9" r="12.5" />
				<circle cx="37.3" cy="171.1" r="12.5" />
				<circle cx="61.2" cy="147.2" r="12.5" />
				<circle cx="61.2" cy="82.7" r="25" />
				<circle cx="85.1" cy="171.1" r="12.5" />
			</g>
			<g fill={rightButtons} {...part('rightButtons')}>
				<path d="M250.5,43.9L254,42.1C244.8,17.6,220.1,0,191.1,0L169.6,0L169.7,3.5L190.9,3.5C217.9,3.5,241.1,20.3,250.5,43.9Z" />
				<path d="M164.5,37.5L158.5,37.5L158.5,31.5L164.5,31.5L164.5,25.5L170.5,25.5L170.5,31.5L176.5,31.5L176.5,37.5L170.5,37.5L170.5,43.5L164.5,43.5L164.5,37.5Z" />
				<circle cx="198.8" cy="107.7" r="12.5" />
				<circle cx="182.4" cy="230.2" r="12.5" />
				<circle cx="222.7" cy="83.8" r="12.5" />
				<circle cx="198.8" cy="59.9" r="12.5" />
				<circle cx="199.9" cy="170.5" r="25" />
				<circle cx="174.9" cy="83.8" r="12.5" />
			</g>
		</svg>
	);
};
export default JoyConPairController;
