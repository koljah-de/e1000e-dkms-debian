// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2019 Intel Corporation. */

#include "e1000.h"
#include "i225.h"
#include "mac.h"
#include "base.h"

static s32 e1000_set_pcie_completion_timeout(struct e1000_hw *hw);
static s32 e1000_reset_hw_82575(struct e1000_hw *hw);
static s32 e1000_setup_copper_link_82575(struct e1000_hw *hw);
static s32 e1000_init_mac_params_base(struct e1000_hw *hw);
static s32 e1000_init_phy_params_base(struct e1000_hw *hw);
static s32 e1000_acquire_phy_82575(struct e1000_hw *hw);
static void e1000_release_phy_82575(struct e1000_hw *hw);
static void e1000_power_down_phy_copper_base(struct e1000_hw *hw);

/**
 *  e1000_set_pcie_completion_timeout - set pci-e completion timeout
 *  @hw: pointer to the HW structure
 *
 *  The defaults for 82575 and 82576 should be in the range of 50us to 50ms,
 *  however the hardware default for these parts is 500us to 1ms which is less
 *  than the 10ms recommended by the pci-e spec.  To address this we need to
 *  increase the value to either 10ms to 200ms for capability version 1 config,
 *  or 16ms to 55ms for version 2.
 **/
static s32 e1000_set_pcie_completion_timeout(struct e1000_hw *hw)
{
	u32 gcr = er32(GCR);
	s32 ret_val = 0;

	/* only take action if timeout value is defaulted to 0 */
	if (gcr & E1000_GCR_CMPL_TMOUT_MASK)
		goto out;

	/*
	 * if capababilities version is type 1 we can write the
	 * timeout of 10ms to 200ms through the GCR register
	 */
	if (!(gcr & E1000_GCR_CAP_VER2)) {
		gcr |= E1000_GCR_CMPL_TMOUT_10ms;
		goto out;
	}

out:
	/* disable completion timeout resend */
	gcr &= ~E1000_GCR_CMPL_TMOUT_RESEND;

	ew32(GCR, gcr);
	return ret_val;
}

/**
 *  e1000_reset_hw_82575 - Reset hardware
 *  @hw: pointer to the HW structure
 *
 *  This resets the hardware into a known state.
 **/
static s32 e1000_reset_hw_82575(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 ret_val;

	/*
	 * Prevent the PCI-E bus from sticking if there is no TLP connection
	 * on the last TLP read/write transaction when MAC is reset.
	 */
	ret_val = e1000e_disable_pcie_master(hw);
	if (ret_val)
		e_dbg("PCI-E Master disable polling has failed.\n");

	/* set the completion timeout for interface */
	ret_val = e1000_set_pcie_completion_timeout(hw);
	if (ret_val)
		e_dbg("PCI-E Set completion timeout has failed.\n");

	e_dbg("Masking off all interrupts\n");
	ew32(IMC, 0xffffffff);

	ew32(RCTL, 0);
	ew32(TCTL, E1000_TCTL_PSP);
	e1e_flush();

	usleep_range(10000, 20000);

	ctrl = er32(CTRL);

	e_dbg("Issuing a global reset to MAC\n");
	ew32(CTRL, ctrl | E1000_CTRL_RST);

	ret_val = e1000e_get_auto_rd_done(hw);
	if (ret_val) {
		/*
		 * When auto config read does not complete, do not
		 * return with an error. This can happen in situations
		 * where there is no eeprom and prevents getting link.
		 */
		e_dbg("Auto Read Done did not complete\n");
	}

	/* If EEPROM is not present, run manual init scripts */
	if (!(er32(EECD) & E1000_EECD_PRES))
		e1000_reset_init_script_82575(hw);

	/* Clear any pending interrupt events. */
	ew32(IMC, 0xffffffff);
	er32(ICR);

	/* Install any alternate MAC address into RAR0 */
	ret_val = e1000_check_alt_mac_addr_generic(hw);

	return ret_val;
}

/**
 *  e1000_init_nvm_params_base - Init NVM func ptrs.
 *  @hw: pointer to the HW structure
 **/
s32 e1000_init_nvm_params_base(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = er32(EECD);
	u16 size;

	size = (u16)((eecd & E1000_EECD_SIZE_EX_MASK) >>
		     E1000_EECD_SIZE_EX_SHIFT);
	/*
	 * Added to a constant, "size" becomes the left-shift value
	 * for setting word_size.
	 */
	size += NVM_WORD_SIZE_BASE_SHIFT;

	/* Just in case size is out of range, cap it to the largest
	 * EEPROM size supported
	 */
	if (size > 15)
		size = 15;

	nvm->word_size = 1 << size;
	nvm->opcode_bits = 8;
	nvm->delay_usec = 1;
	switch (nvm->override) {
	case e1000_nvm_override_spi_large:
		nvm->page_size = 32;
		nvm->address_bits = 16;
		break;
	case e1000_nvm_override_spi_small:
		nvm->page_size = 8;
		nvm->address_bits = 8;
		break;
	default:
		nvm->page_size = eecd & E1000_EECD_ADDR_BITS ? 32 : 8;
		nvm->address_bits = eecd & E1000_EECD_ADDR_BITS ? 16 : 8;
		break;
	}

	nvm->type = e1000_nvm_eeprom_spi;

	if (nvm->word_size == (1 << 15))
		nvm->page_size = 128;

	/* Function Pointers */
	nvm->ops.acquire = e1000_acquire_nvm_82575;
	nvm->ops.release = e1000_release_nvm_82575;
	if (nvm->word_size < (1 << 15))
		nvm->ops.read = e1000e_read_nvm_eerd;
	else
		nvm->ops.read = e1000_read_nvm_spi;

	nvm->ops.write = e1000e_write_nvm_spi;
	nvm->ops.validate = e1000e_validate_nvm_checksum_generic;
	nvm->ops.update = e1000e_update_nvm_checksum_generic;
	nvm->ops.valid_led_default = e1000_valid_led_default_82575;

	/* override generic family function pointers for specific descendants */
	switch (hw->mac.type) {
	case e1000_i350:
	case e1000_i354:
		nvm->ops.validate = e1000_validate_nvm_checksum_i350;
		nvm->ops.update = e1000_update_nvm_checksum_i350;
		break;
	default:
		break;
	}

	return 0;
}

/**
 *  e1000_setup_copper_link_82575 - Configure copper link settings
 *  @hw: pointer to the HW structure
 *
 *  Configures the link for auto-neg or forced speed and duplex.  Then we check
 *  for link, once link is established calls to configure collision distance
 *  and flow control are called.
 **/
static s32 e1000_setup_copper_link_82575(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 ret_val;

	ctrl = er32(CTRL);
	ctrl |= E1000_CTRL_SLU;
	ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	ew32(CTRL, ctrl);

	ret_val = e1000_setup_serdes_link_82575(hw);
	if (ret_val)
		goto out;

	if (e1000_sgmii_active_82575(hw)) {
		/* allow time for SFP cage time to power up phy */
		msleep(300);

		ret_val = e1000_phy_hw_reset(hw);
		if (ret_val) {
			e_dbg("Error resetting the PHY.\n");
			goto out;
		}
	}
	switch (hw->phy.type) {
	case e1000_phy_m88:
		switch (hw->phy.id) {
		default:
			ret_val = e1000e_copper_link_setup_m88(hw);
			break;
		}
		break;
	case e1000_phy_igp_3:
		ret_val = e1000e_copper_link_setup_igp(hw);
		break;
	default:
		ret_val = -E1000_ERR_PHY;
		break;
	}

	if (ret_val)
		goto out;

	ret_val = e1000e_setup_copper_link(hw);
out:
	return ret_val;
}

/**
 *  e1000_init_mac_params_base - Init MAC func ptrs.
 *  @hw: pointer to the HW structure
 **/
static s32 e1000_init_mac_params_base(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	struct e1000_dev_spec_82575 *dev_spec = &hw->dev_spec.e82575;

	/* Derives media type */
	e1000_get_media_type_82575(hw);
	/* Set mta register count */
	mac->mta_reg_count = 128;
	/* Set uta register count */
	mac->uta_reg_count = (hw->mac.type == e1000_82575) ? 0 : 128;
	/* Set rar entry count */
	mac->rar_entry_count = E1000_RAR_ENTRIES_82575;
	if (mac->type == e1000_82576)
		mac->rar_entry_count = E1000_RAR_ENTRIES_82576;
	/* FWSM register */
	mac->has_fwsm = true;
	/* ARC supported; valid only if manageability features are enabled. */
	mac->arc_subsystem_valid = !!(er32(FWSM) & E1000_FWSM_MODE_MASK);

	/* Function pointers */

	/* bus type/speed/width */
	mac->ops.get_bus_info = e1000e_get_bus_info_pcie;
	/* reset */
	mac->ops.reset_hw = e1000_reset_hw_82575;
	/* hw initialization */
	mac->ops.init_hw = e1000_init_hw_82575;
	/* link setup */
	mac->ops.setup_link = e1000e_setup_link_generic;
	/* physical interface link setup */
	mac->ops.setup_physical_interface =
	    (hw->phy.media_type == e1000_media_type_copper)
	    ? e1000_setup_copper_link_82575 : e1000_setup_serdes_link_82575;
	/* physical interface shutdown */
	mac->ops.shutdown_serdes = e1000_shutdown_serdes_link_82575;
	/* physical interface power up */
	mac->ops.power_up_serdes = e1000_power_up_serdes_link_82575;
	/* check for link */
	mac->ops.check_for_link = e1000_check_for_link_82575;
	/* read mac address */
	mac->ops.read_mac_addr = e1000_read_mac_addr_82575;
	/* configure collision distance */
	mac->ops.config_collision_dist = e1000_config_collision_dist_82575;
	/* multicast address update */
	mac->ops.update_mc_addr_list = e1000e_update_mc_addr_list_generic;
	if (hw->mac.type == e1000_i350 || mac->type == e1000_i354) {
		/* writing VFTA */
		mac->ops.write_vfta = e1000_write_vfta_i350;
		/* clearing VFTA */
		mac->ops.clear_vfta = e1000_clear_vfta_i350;
	} else {
		/* writing VFTA */
		mac->ops.write_vfta = e1000_write_vfta_generic;
		/* clearing VFTA */
		mac->ops.clear_vfta = e1000_clear_vfta_generic;
	}
	/* ID LED init */
	mac->ops.id_led_init = e1000e_id_led_init_generic;
	/* blink LED */
	mac->ops.blink_led = e1000e_blink_led_generic;
	/* setup LED */
	mac->ops.setup_led = e1000e_setup_led_generic;
	/* cleanup LED */
	mac->ops.cleanup_led = e1000e_cleanup_led_generic;
	/* turn on/off LED */
	mac->ops.led_on = e1000e_led_on_generic;
	mac->ops.led_off = e1000e_led_off_generic;
	/* clear hardware counters */
	mac->ops.clear_hw_cntrs = e1000_clear_hw_cntrs_82575;
	/* link info */
	mac->ops.get_link_up_info = e1000_get_link_up_info_82575;
	/* acquire SW_FW sync */
	mac->ops.acquire_swfw_sync = e1000_acquire_swfw_sync_82575;
	mac->ops.release_swfw_sync = e1000_release_swfw_sync_82575;
	/* set lan id for port to determine which phy lock to use */
	hw->mac.ops.set_lan_id(hw);

	return 0;
}

/**
 *  e1000_init_phy_params_base - Init PHY func ptrs.
 *  @hw: pointer to the HW structure
 **/
static s32 e1000_init_phy_params_base(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val = 0;
	u32 ctrl_ext;

	if (hw->phy.media_type != e1000_media_type_copper) {
		phy->type = e1000_phy_none;
		goto out;
	}

	phy->ops.power_up = e1000_power_up_phy_copper;
	phy->ops.power_down = e1000_power_down_phy_copper_base;

	phy->autoneg_mask = AUTONEG_ADVERTISE_SPEED_DEFAULT;

	phy->reset_delay_us = 100;

	phy->ops.acquire = e1000_acquire_phy_base;
	phy->ops.check_reset_block = e1000e_check_reset_block_generic;
	phy->ops.commit = e1000e_phy_sw_reset;
	phy->ops.get_cfg_done = e1000_get_cfg_done_82575;
	phy->ops.release = e1000_release_phy_base;

	ctrl_ext = er32(CTRL_EXT);

	if (e1000_sgmii_active_82575(hw)) {
		phy->ops.reset = e1000_phy_hw_reset_sgmii_82575;
		ctrl_ext |= E1000_CTRL_I2C_ENA;
	} else {
		phy->ops.reset = e1000e_phy_hw_reset_generic;
		ctrl_ext &= ~E1000_CTRL_I2C_ENA;
	}

	ew32(CTRL_EXT, ctrl_ext);
	if (e1000_sgmii_active_82575(hw) && !e1000_sgmii_uses_mdio_82575(hw)) {
		phy->ops.read_reg = e1000_read_phy_reg_sgmii_82575;
		phy->ops.write_reg = e1000_write_phy_reg_sgmii_82575;
	} else {
		switch (hw->mac.type) {
		default:
			phy->ops.read_reg = e1000e_read_phy_reg_igp;
			phy->ops.write_reg = e1000e_write_phy_reg_igp;
		}
	}

	/* Set phy->phy_addr and phy->id. */
	ret_val = e1000_get_phy_id_82575(hw);

	/* Verify phy id and set remaining function pointers */
	switch (phy->id) {
	case M88E1111_I_PHY_ID:
		phy->type = e1000_phy_m88;
		phy->ops.check_polarity = e1000_check_polarity_m88;
		phy->ops.get_info = e1000e_get_phy_info_m88;
		phy->ops.get_cable_length = e1000e_get_cable_length_m88;
		phy->ops.force_speed_duplex = e1000e_phy_force_speed_duplex_m88;
		break;
	case IGP03E1000_E_PHY_ID:
	case IGP04E1000_E_PHY_ID:
		phy->type = e1000_phy_igp_3;
		phy->ops.check_polarity = e1000_check_polarity_igp;
		phy->ops.get_info = e1000e_get_phy_info_igp;
		phy->ops.get_cable_length = e1000e_get_cable_length_igp_2;
		phy->ops.force_speed_duplex = e1000e_phy_force_speed_duplex_igp;
		phy->ops.set_d0_lplu_state = e1000_set_d0_lplu_state_82575;
		phy->ops.set_d3_lplu_state = e1000e_set_d3_lplu_state;
		break;
	default:
		ret_val = -E1000_ERR_PHY;
		goto out;
	}

out:
	return ret_val;
}

/**
 *  e1000_acquire_phy_base - Acquire rights to access PHY
 *  @hw: pointer to the HW structure
 *
 *  Acquire access rights to the correct PHY.
 **/
static s32 e1000_acquire_phy_base(struct e1000_hw *hw)
{
	u16 mask = E1000_SWFW_PHY0_SM;

	if (hw->bus.func == E1000_FUNC_1)
		mask = E1000_SWFW_PHY1_SM;

	return hw->mac.ops.acquire_swfw_sync(hw, mask);
}

/**
 *  e1000_release_phy_base - Release rights to access PHY
 *  @hw: pointer to the HW structure
 *
 *  A wrapper to release access rights to the correct PHY.
 **/
static void e1000_release_phy_base(struct e1000_hw *hw)
{
	u16 mask = E1000_SWFW_PHY0_SM;

	if (hw->bus.func == E1000_FUNC_1)
		mask = E1000_SWFW_PHY1_SM;

	hw->mac.ops.release_swfw_sync(hw, mask);
}

/**
 *  e1000_init_hw_82575 - Initialize hardware
 *  @hw: pointer to the HW structure
 *
 *  This inits the hardware readying it for operation.
 **/
s32 e1000_init_hw_82575(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	u16 i, rar_count = mac->rar_entry_count;

	/* Initialize identification LED */
	ret_val = mac->ops.id_led_init(hw);
	if (ret_val) {
		e_dbg("Error initializing identification LED\n");
		/* This is not fatal and we should not stop init due to this */
	}

	/* Disabling VLAN filtering */
	e_dbg("Initializing the IEEE VLAN\n");
	mac->ops.clear_vfta(hw);

	/* Setup the receive address */
	e1000e_init_rx_addrs(hw, rar_count);

	/* Zero out the Multicast HASH table */
	e_dbg("Zeroing the MTA\n");
	for (i = 0; i < mac->mta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_MTA, i, 0);

	/* Zero out the Unicast HASH table */
	e_dbg("Zeroing the UTA\n");
	for (i = 0; i < mac->uta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_UTA, i, 0);

	/* Setup link and flow control */
	ret_val = mac->ops.setup_link(hw);

	/* Set the default MTU size */
	hw->dev_spec.e82575.mtu = 1500;

	/*
	 * Clear all of the statistics registers (clear on read).  It is
	 * important that we do this after we have tried to establish link
	 * because the symbol error count will increment wildly if there
	 * is no link.
	 */
	e1000_clear_hw_cntrs_82575(hw);

	return ret_val;
}

/**
 * e1000_power_down_phy_copper_base - Remove link during PHY power down
 * @hw: pointer to the HW structure
 *
 * In the case of a PHY power down to save power, or to turn off link during a
 * driver unload, or wake on lan is not enabled, remove the link.
 **/
static void e1000_power_down_phy_copper_base(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;

	if (!(phy->ops.check_reset_block))
		return;

	/* If the management interface is not enabled, then power down */
	if (!
	    (e1000e_enable_mng_pass_thru(hw) || phy->ops.check_reset_block(hw)))
		e1000_power_down_phy_copper(hw);

	return;
}

/**
 *  e1000_rx_fifo_flush_base - Clean Rx FIFO after Rx enable
 *  @hw: pointer to the HW structure
 *
 *  After Rx enable, if manageability is enabled then there is likely some
 *  bad data at the start of the FIFO and possibly in the DMA FIFO.  This
 *  function clears the FIFOs and flushes any packets that came in as Rx was
 *  being enabled.
 **/
void e1000_rx_fifo_flush_base(struct e1000_hw *hw)
{
	u32 rctl, rlpml, rxdctl[4], rfctl, temp_rctl, rx_enabled;
	int i, ms_wait;

	/* disable IPv6 options as per hardware errata */
	rfctl = er32(RFCTL);
	rfctl |= E1000_RFCTL_IPV6_EX_DIS;
	ew32(RFCTL, rfctl);

	if (hw->mac.type != e1000_82575 ||
	    !(er32(MANC) & E1000_MANC_RCV_TCO_EN))
		return;

	/* Disable all Rx queues */
	for (i = 0; i < 4; i++) {
		rxdctl[i] = er32(RXDCTL(i));
		ew32(RXDCTL(i), rxdctl[i] & ~E1000_RXDCTL_QUEUE_ENABLE);
	}
	/* Poll all queues to verify they have shut down */
	for (ms_wait = 0; ms_wait < 10; ms_wait++) {
		usleep_range(1000, 2000);
		rx_enabled = 0;
		for (i = 0; i < 4; i++)
			rx_enabled |= er32(RXDCTL(i));
		if (!(rx_enabled & E1000_RXDCTL_QUEUE_ENABLE))
			break;
	}

	if (ms_wait == 10)
		e_dbg("Queue disable timed out after 10ms\n");

	/* Clear RLPML, RCTL.SBP, RFCTL.LEF, and set RCTL.LPE so that all
	 * incoming packets are rejected.  Set enable and wait 2ms so that
	 * any packet that was coming in as RCTL.EN was set is flushed
	 */
	ew32(RFCTL, rfctl & ~E1000_RFCTL_LEF);

	rlpml = er32(RLPML);
	ew32(RLPML, 0);

	rctl = er32(RCTL);
	temp_rctl = rctl & ~(E1000_RCTL_EN | E1000_RCTL_SBP);
	temp_rctl |= E1000_RCTL_LPE;

	ew32(RCTL, temp_rctl);
	ew32(RCTL, temp_rctl | E1000_RCTL_EN);
	e1e_flush();
	usleep_range(2000, 4000);

	/* Enable Rx queues that were previously enabled and restore our
	 * previous state
	 */
	for (i = 0; i < 4; i++)
		ew32(RXDCTL(i), rxdctl[i]);
	ew32(RCTL, rctl);
	e1e_flush();

	ew32(RLPML, rlpml);
	ew32(RFCTL, rfctl);

	/* Flush receive errors generated by workaround */
	er32(ROC);
	er32(RNBC);
	er32(MPC);
}
