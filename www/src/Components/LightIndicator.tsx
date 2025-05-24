import { useDraggable } from '@dnd-kit/core';

type LightIndicatorProps = {
	id: string;
	x: number;
	y: number;
	cellWidth: number;
};

const lightbulb = (
	<svg
		xmlns="http://www.w3.org/2000/svg"
		width="32"
		height="32"
		fill="currentColor"
		viewBox="0 0 16 16"
		style={{ filter: 'drop-shadow(0 4px 16px rgba(255, 255, 200, 0.7))' }}
	>
		<path d="M2 6a6 6 0 1 1 10.174 4.31c-.203.196-.359.4-.453.619l-.762 1.769A.5.5 0 0 1 10.5 13h-5a.5.5 0 0 1-.46-.302l-.761-1.77a2 2 0 0 0-.453-.618A5.98 5.98 0 0 1 2 6m3 8.5a.5.5 0 0 1 .5-.5h5a.5.5 0 0 1 0 1l-.224.447a1 1 0 0 1-.894.553H6.618a1 1 0 0 1-.894-.553L5.5 15a.5.5 0 0 1-.5-.5" />
	</svg>
);

export function LightIndicator({ id, cellWidth, y, x }: LightIndicatorProps) {
	const { attributes, isDragging, listeners, setNodeRef, transform } =
		useDraggable({ id });

	return (
		<div
			{...listeners}
			aria-label="Draggable Light Indicator"
			ref={setNodeRef}
			style={{
				width: cellWidth,
				height: cellWidth,
				transform: `translate(${transform?.x ?? 0}px, ${transform?.y ?? 0}px) scale(${
					isDragging ? 2 : 1.5
				})`,
				justifyContent: 'center',
				alignItems: 'center',
				display: 'flex',
				left: x * cellWidth,
				top: y * cellWidth,
				position: 'absolute',
				appearance: 'none',
				background: 'none',
				border: 'none',
				cursor: 'grab',
				clear: 'both',
			}}
		>
			{lightbulb}
		</div>
	);
}
