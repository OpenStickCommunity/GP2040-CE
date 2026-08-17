import { useEffect, useRef, useState } from "react";

export const useGetContainerDimensions = () => {
	const [dimensions, setDimensions] = useState({ height: 0, width: 0 });
	const containerRef = useRef<HTMLDivElement | null>(null);

	useEffect(() => {
		const resizeObserver = new ResizeObserver((event) => {
			setDimensions({
				height: event[0].contentBoxSize[0].blockSize,
				width: event[0].contentBoxSize[0].inlineSize,
			});
		});

		const node = containerRef.current;
		if (node) resizeObserver.observe(node);

		return () => {
			if (node) resizeObserver.unobserve(node);
			resizeObserver.disconnect();
		};
	}, [containerRef.current]); // Re-run if ref changes

	return { dimensions, containerRef };
};
