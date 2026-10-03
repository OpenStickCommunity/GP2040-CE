import type { AnyObject, Flags, Maybe } from 'yup';

// Make sure we have custom methods registered via yup.addMethod
/* eslint-disable @typescript-eslint/no-unused-vars */
declare module 'yup' {
	interface NumberSchema<
		TType extends Maybe<number> = number | undefined,
		TContext = AnyObject,
		TDefault = undefined,
		TFlags extends Flags = '',
	> {
		checkUsedPins(): this;
		validatePinWhenValue(name: string): this;
		validatePinWhenEqualTo(
			name: string,
			compareName: string,
			compareValue: number,
		): this;
		validateNumberWhenValue(name: string): this;
		validateSelectionWhenValue(
			name: string,
			choices: { value: number }[],
		): this;
		validateMinWhenEqualTo(
			name: string,
			compareValue: number,
			min: number,
		): this;
		validateRangeWhenValue(name: string, min: number, max: number): this;
	}

	interface StringSchema<
		TType extends Maybe<string> = string | undefined,
		TContext = AnyObject,
		TDefault = undefined,
		TFlags extends Flags = '',
	> {
		validateColor(): this;
		validateUSBHexID(): this;
	}
}
