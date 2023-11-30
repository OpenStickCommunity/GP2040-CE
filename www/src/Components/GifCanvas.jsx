import React, { useEffect, useState, useRef, useCallback } from 'react'
import { useTranslation } from 'react-i18next';
import { parseGIF, decompressFrames } from 'gifuct-js'
import chunk from 'lodash/chunk';

const GifCanvas = ({ startingValue, onChange }) => {
    const [frames, setFrames] = useState(null)
    const [framesImageData, setFramesImageData] = useState(null)
    const [framesBytes, setFramesBytes] = useState(null)
    const [currentFrame, setCurrentFrame] = useState(0)
    const [canvasContext, setCanvasContext] = useState(null)
    const [inverted, setInverted] = useState(false)
    const [darkBackground, setDarkBackground] = useState(false)
    const [contrast, setContrast] = useState(123)
    const [invisibleCanvas, setInvisibleCanvas] = useState(null)
    const [invisibleCanvasContext, setInvisibleCanvasContext] = useState(null)
    const canvasRef = useRef()
    const rangeRef = useRef()
    const { t } = useTranslation('')

    const resizeAndAdjustColorForFrame = useCallback((frame) => {
        if (invisibleCanvas.height === 0 || invisibleCanvas.width === 0) {
            invisibleCanvas.height = 64
            invisibleCanvas.width = 128
        }
        const patch = new Uint8ClampedArray(frame.patch)
        const { width: canvasWidth, height: canvasHeight} = canvasContext.canvas
        const { width: imgWidth, height: imgHeight} = frame.dims
        const imgRatio = imgWidth / imgHeight
        const canvasRatio = canvasWidth / canvasHeight
        var offsetX = 0
        var offsetY = 0
        var finalImageWidth = canvasWidth
        var finalImageHeight = canvasHeight

        // Convert to monochrome
        for (var i = 0; i < patch.length; i = i + 4) {
            const avg = (patch[i] + patch[i + 1] + patch[i + 2]) / 3
            const color = inverted
                ? avg > contrast ? 0 : 255
                : avg <= contrast ? 0 : 255
 
            patch[i] = color
            patch[i + 1] = color
            patch[i + 2] = color
            patch[i + 3] = 255
        }

        // Resize to fit into canvas
        if (imgRatio < canvasRatio) {
            finalImageWidth = canvasWidth * imgRatio / canvasRatio
            offsetX = (canvasWidth - finalImageWidth) / 2
        }
        else if (imgRatio > canvasRatio) {
            finalImageHeight = canvasHeight - canvasHeight * canvasRatio / imgRatio
            offsetY = (canvasHeight - finalImageHeight) / 2
        }

        const imageData = new ImageData(patch, imgWidth, imgHeight)
        invisibleCanvasContext.putImageData(imageData, 0, 0)
        invisibleCanvasContext.drawImage(invisibleCanvas, 0, 0, imgWidth, imgHeight, offsetX, offsetY, finalImageWidth, finalImageHeight)
        invisibleCanvasContext.rect(0, 0, 128, offsetY)
        invisibleCanvasContext.rect(0, 0, offsetX, 64)
        invisibleCanvasContext.rect(0, offsetY + finalImageHeight, 128, 64)
        invisibleCanvasContext.rect(offsetX + finalImageWidth, 0, 128, 64)
        invisibleCanvasContext.fillStyle = darkBackground ? '#000000' : '#ffffff'
        invisibleCanvasContext.fill()
    
        return invisibleCanvasContext.getImageData(0, 0, canvasWidth, canvasHeight)
    }, [canvasContext, invisibleCanvasContext, invisibleCanvas, darkBackground, contrast, inverted])

    const createBytesForFrame = useCallback((imgData, delay) => {
        // Set has_delay to true + struct padding
        const bytesArray = [1, 0, 0, 0]

        // Set delay value
        const int32Buffer = new ArrayBuffer(4)
        const int32view = new DataView(int32Buffer)
        int32view.setInt32(0, delay, true)
        bytesArray.push(...new Uint8Array(int32Buffer))

        // Set has_frame to true + struct padding
        bytesArray.push(...[1, 0])

        // Set frame.size value
        const uint16Buffer = new ArrayBuffer(2)
        const uint16view = new DataView(uint16Buffer)
        uint16view.setUint16(0, 1024, true)
        bytesArray.push(...new Uint8Array(uint16Buffer))

        // Set frame.bytes value
        chunk(
            [...new Uint8Array(imgData)].filter((_data, index) => index % 4 === 0),
            8,
        ).forEach((chunks) =>{
            bytesArray.push(chunks.reduce((acc, curr, i) => {
                return acc + ((curr === 255 ? 1 : 0) << (7 - i))
            }, 0))
        })

        return bytesArray
    }, [])

    const toggleInverted = useCallback(() => {
        setInverted(prevState => !prevState)
    }, [])


    const toggleBackground = useCallback(() => {
        setDarkBackground(prevState => !prevState)
    }, [])

    useEffect(() => {
        rangeRef.current.addEventListener('change', e => setContrast(e.target.value))
    }, [])

    useEffect(() => {
        const invisibleCanvas = document.createElement('canvas')
        invisibleCanvas.width = canvasRef.width
        invisibleCanvas.height = canvasRef.height

        setCanvasContext(canvasRef.current.getContext('2d'))
        setInvisibleCanvas(invisibleCanvas)
        setInvisibleCanvasContext(invisibleCanvas.getContext('2d'))
    }, [])

    // Set starting frame
    useEffect(() => {
        /*
        The compiler adds some padding to SplashFrame making so that we have 1036 bytes per frame instead of 1032
        Indexes:
            0       -   has_delay
            4~7     -   delay
            8       -   has_frame
            10~11   -   frame.size
            12~1036 -   frame.bytes
        */
        if (!startingValue) return
        if (startingValue.length % 1036 !== 0) {
            console.warn("Starting value for canvas should be an array of length N * 1036, where N > 0\n", startingValue)
            return
        }
        
        const areCurrentAndStartingBytesEqual =
            !!framesBytes
            && framesBytes.length === startingValue.length
            && framesBytes.reduce((areEqual, value, index) => areEqual && value === startingValue[index], true)

        if (areCurrentAndStartingBytesEqual) return

        const raw_frames = chunk([...new Uint8Array(startingValue)], 1036)
        setFrames(raw_frames.map((raw_frame) => {
            const bitsArray = raw_frame.slice(12).flatMap((a) => {
                const bits = a.toString(2).split('').map(Number)
                const full = Array(8 - bits.length)
                    .fill(0)
                    .concat(bits)
                return full.map((a) => (a === 1 ? 255 : 0))
            });

            // fill up the new array as RGBA
            const rgbaBitsArray = bitsArray.flatMap((x) => [x, x, x, 255])

            // Create a buffer from the array
            const buffer = new ArrayBuffer(4)
            const view = new DataView(buffer)

            // Set the bytes in little-endian order (most significant byte first)
            for (let i = 0; i < 4; i++) {
                view.setUint8(i, raw_frame[4 + i])
            }

            // Get the signed 32-bit integer value
            const delay = view.getInt32(0, true)

            return {
                patch: new Uint8ClampedArray(rgbaBitsArray),
                dims: {
                    height: 64,
                    width: 128
                },
                delay,
            }
        }))
    }, [startingValue])

    useEffect(() => {
        if (!canvasContext) return
        if (!framesImageData || framesImageData.length === 0) return
        if (!frames) return

        const { width, height}  = canvasContext.canvas
        const imageData = framesImageData[currentFrame]
        const frame = frames[currentFrame]

        if (!framesImageData) return
        if (!frame)return

        canvasContext.putImageData(imageData, 0, 0, 0, 0, width, height)
         
        const timeout = setTimeout(() => {
            const nextFrame = currentFrame === frames.length - 1
                ? 0
                : currentFrame + 1
            setCurrentFrame(nextFrame)
        }, frame.delay)

        return () => {
            clearTimeout(timeout)
        }
    }, [canvasContext, frames, framesImageData, currentFrame])

    useEffect(() => {
        if (canvasContext == null) return
        if (frames == null) return

        const framesData = []
        const bytesArray = []

        frames.forEach((frame) => {
            const frameData = resizeAndAdjustColorForFrame(frame)
            framesData.push(frameData)
            bytesArray.push(...createBytesForFrame(frameData.data, frame.delay)) 
        })
        setCurrentFrame(0)
        setFramesImageData(framesData)
        setFramesBytes(bytesArray)
        onChange(bytesArray)
    }, [frames, canvasContext, resizeAndAdjustColorForFrame, createBytesForFrame, setFramesBytes])

    const onImageAdd = (ev) => {
        var file = ev.target.files[0]
        var fr = new FileReader()
        fr.onload = () => {
            var gif = parseGIF(fr.result)
            var frames = decompressFrames(gif, true)
            const { width, height } = frames[0].dims
            invisibleCanvas.width = Math.max(width, canvasRef.current.width)
            invisibleCanvas.height = Math.max(height, canvasRef.current.height)
            setFrames(frames)
        }
        fr.readAsArrayBuffer(file)
    }

    return (
        <div style={{ display: 'flex', alignItems: 'center' }}>
            <canvas
                ref={canvasRef}
                width="128"
                height="64"
                style={{ background: 'black' }}
            />
            <div style={{ marginLeft: '11px' }}>
                <input
                    type="file"
                    id="image-input"
                    accept="image/gif"
                    onChange={onImageAdd}
                />
                <br />
                <input
                    type="checkbox"
                    checked={inverted}
                    onChange={toggleInverted}
                />{' '}
                {t('DisplayConfig:form.inverted-label')}
                <br />
                <input
                    type="checkbox"
                    checked={darkBackground}
                    onChange={toggleBackground}
                />{' '}
                {t('DisplayConfig:form.dark-background')}
                <br />
                <input
                    ref={rangeRef}
                    type="range"
                    min="0"
                    max="255"
                />{' '}
                {t('DisplayConfig:form.contrast')}
            </div>
        </div>
    )
}

export default GifCanvas