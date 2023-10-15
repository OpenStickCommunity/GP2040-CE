import React, { useEffect, useState } from 'react';
import Select from 'react-select';
import styles from '@site/src/components/labelselector.module.css';

const SELECT_VALUE_KEY = "SelectedInputLabel";

const inputLabels = [
    { value: "GP2040", label: "GP2040"},
    { value: "XInput", label: "XInput"},
    { value: "DirectInput", label: "DirectInput"},
    { value: "Nintendo Switch", label: "Nintendo Switch"},
    { value: "PS3", label: "PS3"},
    { value: "PS4", label: "PS4"},
    { value: "Arcade", label: "Arcade"},
]

const labelData = {
    "GP2040": { "name": "GP2040", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "B1", "B2": "B2", "B3": "B3", "B4": "B4", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "S1", "S2": "S2", "L3": "L3", "R3": "R3", "A1": "A1", "A2": "A2" },
    "XInput": { "name": "XInput", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "A", "B2": "B", "B3": "X", "B4": "Y", "L1": "LB", "R1": "RB", "L2": "LT", "R2": "RT", "S1": "Back", "S2": "Start", "L3": "LS", "R3": "RS", "A1": "Guide", "A2": "(A2)" },
    "DirectInput": { "name": "DirectInput", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "2", "B2": "3", "B3": "1", "B4": "4", "L1": "5", "R1": "6", "L2": "7", "R2": "8", "S1": "9", "S2": "10", "L3": "11", "R3": "12", "A1": "13", "A2": "14" },
    "Nintendo Switch": { "name": "Nintendo Switch", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "B", "B2": "A", "B3": "Y", "B4": "X", "L1": "L", "R1": "R", "L2": "ZL", "R2": "ZR", "S1": "Minus", "S2": "Plus", "L3": "LS", "R3": "RS", "A1": "Home", "A2": "Capture" },
    "PS4":    { "name": "PS4", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "Cross", "B2": "Circle", "B3": "Square", "B4": "Triangle", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "Share", "S2": "Options", "L3": "L3", "R3": "R3", "A1": "PS", "A2": "Touchpad" },
    "PS3":    { "name": "PS3", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "Cross", "B2": "Circle", "B3": "Square", "B4": "Triangle", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "Select", "S2": "Start", "L3": "L3", "R3": "R3", "A1": "PS", "A2": "(A2)" },
    "Arcade": { "name": "Arcade", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "K1", "B2": "K2", "B3": "P1", "B4": "P2", "L1": "P4", "R1": "P3", "L2": "K4", "R2": "K3", "S1": "Select", "S2": "Start", "L3": "LS", "R3": "RS", "A1": "Home", "A2": "(A2)" }
}

const customStyles = {
    option: (styles, { data, isDisabled, isSelected }) => {
        const color = 0x000000;
        return {
            ...styles,
            color: isDisabled
                ? '#ccc'
                : isSelected
                ? 'white'
                : 'black',        
            ':active': {
                ...styles[':active'],
                backgroundColor: !isDisabled
                ? isSelected
                    ? data.color
                    : color
                : undefined,
            },
        };
    }
}

export default function InputLabelSelector() {
    const [inputLabel, setInputLabel] = useState("");
    const handleChange = (s) => {
        if (typeof window !== 'undefined') {
            localStorage.setItem(SELECT_VALUE_KEY, JSON.stringify(s));
            window.dispatchEvent(new Event('inputLabelChange'))
        }
        setInputLabel(s);
    };

    useEffect(() => {
        if (typeof window !== 'undefined') {
            const lastSelected = JSON.parse(
                localStorage.getItem(SELECT_VALUE_KEY) ?? "[]"
            );
            setInputLabel(lastSelected);
        }
    }, []);

    return (
            <Select 
                value={inputLabel}
                className={styles.labelSelector} 
                options={inputLabels}
                onChange={handleChange}
                styles={customStyles}
                />
    )
  }

export function Hotkey(props) {
    const [hotkeyCombo, setHotkeyCombo] = useState(null)
    let inputLabel = { value: "GP2040", label: "GP2040"};
    if (typeof window !== 'undefined') {
        inputLabel = JSON.parse(localStorage.getItem(SELECT_VALUE_KEY));
    }

    const createNewHotkey = () => {
        let newHotkeyCombo = "";
        props.buttons.forEach(function(input, index, array) {
            let newInput = labelData[inputLabel.value][input]
            if(index === array.length - 1) {
                newHotkeyCombo += newInput
            } else {
                newHotkeyCombo += newInput + " + ";
            }
        });
    
        return newHotkeyCombo;
    }
    

    useEffect(() => {
        setHotkeyCombo(createNewHotkey())  

        window.addEventListener('inputLabelChange', () => {
            if (typeof window !== 'undefined') {
                inputLabel = JSON.parse(localStorage.getItem(SELECT_VALUE_KEY))
            }
            let newHotkeyCombo = createNewHotkey();

            setHotkeyCombo(newHotkeyCombo)  
        });

        return () => window.removeEventListener('inputLabelChange', setHotkeyCombo )
    })

    return (
                <code>{hotkeyCombo}</code>
    );
  }