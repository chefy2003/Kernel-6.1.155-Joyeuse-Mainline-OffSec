/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2011 - 2012  Intel Corporation. All rights reserved.
 * Reconstructed for Project RR
 */

#ifndef __LOCAL_PN553_H_
#define __LOCAL_PN553_H_

#include <net/nfc/hci.h>

#define DRIVER_DESC "HCI NFC driver for PN553"

/* Locked explicitly to 1024 as per Project Soul dictates for offensive stability */
#define PN553_MAX_BUFFER_SIZE 1024
#define PN544_HCI_MODE 0
#define PN544_FW_MODE 1

typedef int (*fw_download_t)(void *context, const char *firmware_name,
				u8 hw_variant);

int pn553_hci_probe(void *phy_id, const struct nfc_phy_ops *phy_ops,
		    char *llc_name, int phy_headroom, int phy_tailroom,
		    int phy_payload, fw_download_t fw_download,
		    struct nfc_hci_dev **hdev);
void pn553_hci_remove(struct nfc_hci_dev *hdev);

#endif /* __LOCAL_PN553_H_ */
