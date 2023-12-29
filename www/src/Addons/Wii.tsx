import { AppContext } from '../Contexts/AppContext';
import React, { useContext, useEffect, useState } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { Button, Form, Row, FormCheck, Tab, Tabs, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { I2C_BLOCKS, PERIPHERAL_DEVICES } from '../Data/Peripherals';
import { BUTTON_MASKS } from '../Data/Buttons';

import WebApi, { baseWiiControls, basePeripheralMapping } from '../Services/WebApi';

export const wiiScheme = {
	WiiExtensionAddonEnabled: yup
		.number()
		.required()
		.label('Wii Extensions Enabled'),
	wiiExtensionBlock: yup
		.number()
		.required()
		.label('WiiExtension I2C Block')
		.validateSelectionWhenValue('WiiExtensionAddonEnabled', I2C_BLOCKS),
};

export const wiiState = {
	WiiExtensionAddonEnabled: 0,
	wiiExtensionBlock: 0,
};

const WII_EXTENSION_CONTROLS = [
    { id: 'Nunchuk',   value: 0, inputs: { analog: [ { id: 'Stick', axes: [ { id: 'X', axis: 0 }, { id: 'Y', axis: 1 } ] } ], digital: [ { id: 'C' }, { id: 'Z' } ] } },
    { id: 'Classic',   value: 1, inputs: { analog: [ { id: 'LeftStick', axes: [ { id: 'X', axis: 0 }, { id: 'Y', axis: 1 } ] }, { id: 'RightStick', axes: [ { id: 'X', axis: 2 }, { id: 'Y', axis: 3 } ] }, { id: 'LeftTrigger', axes: [ { id: 'Trigger', axis: 4 } ] }, { id: 'RightTrigger', axes: [ { id : 'Trigger', axis: 5 } ] } ], digital: [ { id: 'A' }, { id: 'B' }, { id: 'X' }, { id: 'Y' }, { id: 'L' }, { id: 'R' }, { id: 'ZL' }, { id: 'ZR' }, { id: 'Minus' }, { id: 'Home' }, { id: 'Plus' }, { id: 'Up' }, { id: 'Down' }, { id: 'Left' }, { id: 'Right' } ] } },
    { id: 'Guitar',    value: 3, inputs: { analog: [ { id: 'Stick', axes: [ { id: 'X', axis: 0 }, { id: 'Y', axis: 1 } ] }, { id: 'WhammyBar', axes: [ { id: 'Trigger', axis: 4 } ] } ], digital: [ { id: 'Orange' }, { id: 'Red' }, { id: 'Blue' }, { id: 'Green' }, { id: 'Yellow' }, { id: 'Pedal' }, { id: 'Minus' }, { id: 'Plus' }, { id: 'StrumUp' }, { id: 'StrumDown' } ] } },
    { id: 'Drum',      value: 4, inputs: { analog: [ { id: 'Stick', axes: [ { id: 'X', axis: 0 }, { id: 'Y', axis: 1 } ] } ], digital: [ { id: 'Orange' }, { id: 'Red' }, { id: 'Blue' }, { id: 'Green' }, { id: 'Yellow' }, { id: 'Pedal' }, { id: 'Minus' }, { id: 'Plus' } ] } },
    { id: 'Turntable', value: 5, inputs: { analog: [ { id: 'Stick', axes: [ { id: 'X', axis: 0 }, { id: 'Y', axis: 1 } ] }, { id: 'LeftTurntable', axes: [ { id: 'Trigger', axis: 0 } ] }, { id: 'RightTurntable', axes: [ { id: 'Trigger', axis: 2 } ] }, { id: 'Fader', axes: [ { id: 'Trigger', axis: 4 } ] }, { id: 'Effects', axes: [ { id: 'Trigger', axis: 5 } ] } ], digital: [ { id: 'LeftGreen' }, { id: 'LeftRed' }, { id: 'LeftBlue' }, { id: 'RightGreen' }, { id: 'RightRed' }, { id: 'RightBlue' }, { id: 'Euphoria' }, { id: 'Minus' }, { id: 'Plus' } ] } },
    { id: 'Taiko',     value: 6, inputs: { analog: [], digital: [ { id: 'DonLeft' }, { id: 'KatLeft' }, { id: 'DonRight' }, { id: 'KatRight' } ] } },
];

const WII_JOYSTICK_MODES = [
	{ label: 'None', value: 0, options: {'x': 0, 'y': 0} },
	{ label: 'Left Analog', value: 1, options: {'x': 1, 'y': 2} },
	{ label: 'Right Analog', value: 2, options: {'x': 3, 'y': 4} },
	{ label: 'D-Pad', value: 3, options: {'x': 5, 'y': 6} },
];

const Wii = ({ values, errors, handleChange, handleCheckbox, setFieldValue }) => {
	const { t } = useTranslation();
    const [wiiControls, setWiiControls] = useState(baseWiiControls);
    const [selectedControls] = useState(baseWiiControls);
    const { setLoading, getAvailablePeripherals, getSelectedPeripheral } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
            const wc = await WebApi.getWiiControls(setLoading);
			setWiiControls(wc);
		}

		fetchData();
	}, [setWiiControls, selectedControls]);

    const ANALOG_SINGLE_AXIS_MODES = [
        { label: 'None',                 value: 0 },
        { label: 'Left Trigger',         value: 7 },
        { label: 'Right Trigger',        value: 8 },
        { label: 'Left Analog X+ Axis',  value: 9 },
        { label: 'Left Analog X- Axis',  value: 10 },
        { label: 'Left Analog Y+ Axis',  value: 11 },
        { label: 'Left Analog Y- Axis',  value: 12 },
        { label: 'Right Analog X+ Axis', value: 13 },
        { label: 'Right Analog X- Axis', value: 14 },
        { label: 'Right Analog Y+ Axis', value: 15 },
        { label: 'Right Analog Y- Axis', value: 16 },
    ];
    
    const saveWiiOptions = async () => {
        let wiiButtonsMap = Array.from(document.querySelectorAll('#WiiExtensionAddonOptions .wii-buttons'));
        let wiiMap = wiiButtonsMap
            .reduce(
                (o, i) => Object.assign(o, { [i.getAttribute('controlid')+'.button'+i.getAttribute('buttonid')]: parseInt(i.value) }), {});

        let wiiAnalogMap = Array.from(document.querySelectorAll('#WiiExtensionAddonOptions .wii-analogs'));
        wiiMap = wiiAnalogMap
        .reduce(
            (o, i) => {
                let modeID = i.value;
                let axes = i.getAttribute('axiscount') || 2;
                let joyMode = (axes == 2 ? getJoystickMode(modeID) : null);
                if (joyMode && joyMode.options) {
                    let r = o;
                    Object.keys(joyMode.options).forEach(key => {
                        Object.assign(r, { [i.getAttribute('controlid')+'.analog'+i.getAttribute('analogid')+'.'+key+'.axisType']: joyMode.options[key] });
                    });
                    o = r;
                } else {
                    o = Object.assign(o, { [i.getAttribute('controlid')+'.analog'+i.getAttribute('analogid')+'.axisType']: parseInt(i.value) });
                }
                return o;
            }, wiiMap);
    
        try {
            let success = await WebApi.setWiiControls(wiiMap);
    
            if (success) {
    
            } else {
                
            }
        } catch (e) {
        }
    };

    const getJoystickMode = (searchValue: number) => WII_JOYSTICK_MODES.find(({ value }) => parseInt(value) === parseInt(searchValue));

    const setWiiAnalogEntry = (controlID,analogID,axes,e) => {
        let analogEntry = {};
        let modeID = e.target.value;
        let joyMode = (axes == 2 ? getJoystickMode(modeID) : null);
        if (joyMode && joyMode.options) {
            let r = analogEntry;
            Object.keys(joyMode.options).forEach(key => {
                Object.assign(r, { [controlID.toLowerCase()+'.analog'+analogID+'.'+key+'.axisType']: joyMode.options[key] });
            });
            analogEntry = r;
        } else {
            analogEntry[`${controlID.toLowerCase()}.analog${analogID}.axisType`] = e.target.value;
        }

        setWiiControls(controls => ({
            ...wiiControls,
            ...analogEntry
        }));
    };

    const handlePeripheralChange = (e) => {
        let device = getSelectedPeripheral('i2c', e.target.value);
        handleChange(e);
    };

    const getJoystickModeValue = (controlObj, analogObj) => {
        let joystickMode = 0;
        let foundOpt = false;
        WII_JOYSTICK_MODES.forEach(mode => {
            if (!foundOpt) {
                Object.keys(mode.options).forEach(opt => {
                    if (!foundOpt) {
                        let modeEntry = mode.options[opt];
                        let entryID = controlObj.id.toLowerCase()+'.analog'+analogObj.id+'.'+opt+'.axisType';
                        let entry = wiiControls[entryID];
                        if (wiiControls[entryID] == modeEntry) {
                            foundOpt = true;
                            joystickMode = mode.value;
                        }
                    }
                });
            }
        });
        return joystickMode;
    };

	return (
        <Section title={t('WiiAddon:header-text')}>
            <div
                id="WiiExtensionAddonOptions"
                hidden={!(values.WiiExtensionAddonEnabled && getAvailablePeripherals('i2c'))}
            >
                <Row>
                    <Trans
                        ns="WiiAddon"
                        i18nKey="sub-header-text"
                    >
                        <p>
                            Note: If the Display is enabled at the same time, this Addon
                            will be disabled.
                        </p>
                    </Trans>
                </Row>
                <Row className="mb-3">
                    {getAvailablePeripherals('i2c') ?
                    <FormSelect
                        label={t('WiiAddon:block-label')}
                        name="wiiExtensionBlock"
                        className="form-select-sm"
                        groupClassName="col-sm-2 col-md-2 mb-3"
                        value={values.wiiExtensionBlock}
                        error={errors.wiiExtensionBlock}
                        isInvalid={errors.wiiExtensionBlock}
                        onChange={handlePeripheralChange}
                    >
                        {getAvailablePeripherals('i2c').map((o, i) => (
                            <option
                                key={`wiiExtensionI2cBlock-option-${i}`}
                                value={o.value}
                            >
                                {o.label}
                            </option>
                        ))}
                    </FormSelect>
                    : ''}
                </Row>
                <Row className="mb-3">
                    <Tabs
                        defaultActiveKey={`wii${WII_EXTENSION_CONTROLS[0].id}Config`}
                        id="wiiControllerConfig"
                        className="mb-3"
                        fill
                    >
                        {WII_EXTENSION_CONTROLS.map((controlObj, controlID) => (
                            <Tab key={`wii-addon-controller-${controlID}`} eventKey={`wii${controlObj.id}Config`} title={t(`WiiAddon:controller-${controlObj.id.toLowerCase()}`)}>
                                <Row className="mb-3">
                                    {controlObj.inputs.digital?.length > 0 ? <h1>{t('WiiAddon:section-digital')}</h1> : ''}
                                    {controlObj.inputs.digital?.map((buttonObj,buttonID) => (
                                        <div className="col-sm-12 col-md-6 col-lg-2 mb-2" key={`wiiExtensionController${controlObj.id}Digital${buttonID}`}>
                                            <label className="form-label" htmlFor={`wiiExtensionController${controlObj.id}Button${buttonObj.id}`}>{t(`WiiAddon:controller-button-${buttonObj.id.toLowerCase()}`)}</label>
                                            <select className="form-select-sm form-control wii-buttons" controlid={`${controlObj.id.toLowerCase()}`} buttonid={`${buttonObj.id}`} id={`wiiExtensionController${controlObj.id}Button${buttonObj.id}`} value={wiiControls[controlObj.id.toLowerCase()+'.button'+buttonObj.id]} onChange={(e) => setWiiControls((controls) => ({...controls,[`${controlObj.id.toLowerCase()}.button${buttonObj.id}`]: e.target.value,}))}>
                                            {BUTTON_MASKS.map((o, i) => (
                                                <option key={`wiiExtensionController${controlObj.id}Button${buttonObj.id}-option-${i}`} value={o.value}>
                                                    {o.label}
                                                </option>
                                            ))}
                                            </select>
                                        </div>
                                    ))}

                                    {controlObj.inputs.analog?.length > 0 ? <h1>{t('WiiAddon:section-analog')}</h1> : ''}
                                    {controlObj.inputs.analog?.map((analogObj,analogID) => (
                                        <div className="row" key={`analog-input-${analogID}`}>
                                            <div className="col-sm-1 mb-2" key={`wiiExtensionController${controlObj.id}Analog${analogID}`}>
                                                {t(`WiiAddon:controller-analog-${analogObj.id.toLowerCase()}`)}
                                            </div>
                                            <div className="col-sm-11 mb-2" key={`wiiExtensionController${controlObj.id}Analog${analogID}Options`}>
                                                <div className="col-sm-12 col-md-6 col-lg-2 mb-2">
                                                    {analogObj.axes?.length == 1 &&
                                                    <div className="row">
                                                        <select className="form-select-sm form-control wii-analogs" controlid={`${controlObj.id.toLowerCase()}`} analogid={`${analogObj.id}`} axisid={`${analogObj.axes[0].axis}`} axiscount={analogObj.axes?.length} id={`wiiExtensionController${controlObj.id}Analog${analogObj.id}`} value={wiiControls[controlObj.id.toLowerCase()+'.analog'+analogObj.id+'.axisType']} onChange={(e) => setWiiAnalogEntry(controlObj.id, analogObj.id, analogObj.axes?.length, e)}>
                                                            {ANALOG_SINGLE_AXIS_MODES.map((o,i) => (
                                                                <option key={`wiiSingleAxisMode${controlObj.id}${analogID}${i}`} value={o.value}>{o.label}</option>
                                                            ))}
                                                        </select>
                                                    </div>
                                                    }
                                                    {analogObj.axes?.length == 2 &&
                                                    <div className="row">
                                                        <select className="form-select-sm form-control wii-analogs" controlid={`${controlObj.id.toLowerCase()}`} analogid={`${analogObj.id}`} axisid={`${analogObj.axes[0].axis}`} axiscount={analogObj.axes?.length} id={`wiiExtensionController${controlObj.id}Analog${analogObj.id}`} value={getJoystickModeValue(controlObj, analogObj)} onChange={(e) => setWiiAnalogEntry(controlObj.id, analogObj.id, analogObj.axes?.length, e)}>
                                                            {WII_JOYSTICK_MODES.map((o,i) => (
                                                                <option key={`wiiJoystickMode${controlObj.id}${analogID}${i}`} value={o.value}>{o.label}</option>
                                                            ))}
                                                        </select>
                                                    </div>
                                                    }
                                                </div>
                                            </div>
                                        </div>
                                    ))}
                                </Row>
                            </Tab>
                        ))}
                    </Tabs>
                </Row>
                <Row className="mb-3">
                    <div className="col-sm-3 mb-3">
                        <Button type="button" onClick={saveWiiOptions}>
                            {t('WiiAddon:button-save')}
                        </Button>
                    </div>
                </Row>
            </div>
            {getAvailablePeripherals('i2c') ?
            <FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="WiiExtensionButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.WiiExtensionAddonEnabled) && getAvailablePeripherals('i2c')}
				onChange={(e) => {
					handleCheckbox('WiiExtensionAddonEnabled', values);
					handleChange(e);
				}}
			/>
            :
            <FormLabel><Trans ns="PeripheralMapping" i18nKey="peripheral-toggle-unavailable" values={{'name':'I2C'}}><NavLink exact="true" to="/peripheral-mapping">{t('PeripheralMapping:header-text')}</NavLink></Trans></FormLabel>
            }
		</Section>
	);
};

export default Wii;
