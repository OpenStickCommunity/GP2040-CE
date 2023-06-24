/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 * SPDX-FileCopyrightText: Copyright (c) 2023 Project Alpaca (project-alpaca.github.io)
 */

#include "addons/ps4mode.h"
#include "helper.h"
#include "config.pb.h"

#include "ps4_driver.h"

// Call the DS4key constructor
PS4ModeAddon::PS4ModeAddon() : ds4Key() {};

bool PS4ModeAddon::available() {
	const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
	return options.enabled && options.ds4Key.size == sizeof(options.ds4Key.bytes);
}

void PS4ModeAddon::setup() {
	const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
	this->ds4Key.load(options.ds4Key.bytes, true);
}

void PS4ModeAddon::process() {
		// Check to see if the PS4 Authentication needs work
		if ( PS4Data::getInstance().ps4State == PS4State::nonce_ready ) {
			auto nonce = PS4Data::getInstance().nonce_buffer;
			auto& response = PS4Data::getInstance().ps4_auth_buffer;
			if (!this->ds4Key.sign(nonce, response)) {
				// TODO maybe set an error condition here. But what to set?
				return;
			}
			PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready; // signed and ready to party
		}
}
