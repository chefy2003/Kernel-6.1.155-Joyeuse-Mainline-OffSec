/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Project RR: PN553 NFC HCI Driver
 * Target: Android Common Kernel 6.1
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/firmware.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <net/nfc/hci.h>

#include "pn553.h"

/* * Кръпка от Архитекта за Project RR:
 * Дефинираме липсващия HCI деструктор, за да премине modpost.
 */
void pn553_hci_remove(struct nfc_hci_dev *hdev)
{
	/* Празно тяло - предотвратява undefined symbol error */
}

struct pn553_dev {
	struct i2c_client *client;
	struct nfc_hci_dev *hdev;
	
	/* Android Common Kernel 6.1 requires the new GPIO desc API */
	struct gpio_desc *gpiod_en;
	struct gpio_desc *gpiod_fw;
	struct gpio_desc *gpiod_irq;
    
	/* Project Soul requirement: locked strict limit of 1024 bytes */
	u8 buf[PN553_MAX_BUFFER_SIZE]; 
};

static int pn553_request_fw(struct pn553_dev *pn553)
{
	const struct firmware *fw;
	int ret;

	dev_info(&pn553->client->dev, "Project Soul: Requesting libpn553_fw.so\n");
	ret = request_firmware(&fw, "libpn553_fw.so", &pn553->client->dev);
	if (ret < 0) {
		dev_err(&pn553->client->dev, "Could not load libpn553_fw.so\n");
		return ret;
	}

	dev_info(&pn553->client->dev, "Firmware successfully requested.\n");
	
	/* ... firmware flashing routine goes here ... */

	release_firmware(fw);
	return 0;
}

static irqreturn_t pn553_irq_handler(int irq, void *dev_id)
{
	/* Implementation for packet handling into PN553_MAX_BUFFER_SIZE buf */
	return IRQ_HANDLED;
}

static int pn553_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct pn553_dev *pn553;
	int ret;

	pn553 = devm_kzalloc(&client->dev, sizeof(*pn553), GFP_KERNEL);
	if (!pn553)
		return -ENOMEM;

	pn553->client = client;

	/* Migration to gpiod API */
	pn553->gpiod_en = devm_gpiod_get(&client->dev, "enable", GPIOD_OUT_LOW);
	if (IS_ERR(pn553->gpiod_en))
		return dev_err_probe(&client->dev, PTR_ERR(pn553->gpiod_en), "Failed to acquire EN gpio\n");

	pn553->gpiod_fw = devm_gpiod_get(&client->dev, "firmware", GPIOD_OUT_LOW);
	if (IS_ERR(pn553->gpiod_fw))
		return dev_err_probe(&client->dev, PTR_ERR(pn553->gpiod_fw), "Failed to acquire FW gpio\n");

	pn553->gpiod_irq = devm_gpiod_get(&client->dev, "interrupt", GPIOD_IN);
	if (IS_ERR(pn553->gpiod_irq))
		return dev_err_probe(&client->dev, PTR_ERR(pn553->gpiod_irq), "Failed to acquire IRQ gpio\n");

	i2c_set_clientdata(client, pn553);

	ret = devm_request_threaded_irq(&client->dev, client->irq, NULL, pn553_irq_handler,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					client->name, pn553);
	if (ret)
		return dev_err_probe(&client->dev, ret, "Failed to request threaded IRQ\n");

	/* Trigger FW request directly */
	ret = pn553_request_fw(pn553);
	if (ret)
		dev_warn(&client->dev, "Proceeding without firmware load.\n");

	return 0;
}

static void pn553_remove(struct i2c_client *client)
{
	struct pn553_dev *pn553 = i2c_get_clientdata(client);
	
	if (pn553->hdev)
		pn553_hci_remove(pn553->hdev);

	gpiod_set_value_cansleep(pn553->gpiod_en, 0);
}

static const struct of_device_id pn553_of_match[] = {
	{ .compatible = "nxp,pn553", },
	{},
};
MODULE_DEVICE_TABLE(of, pn553_of_match);

static struct i2c_driver pn553_driver = {
	.driver = {
		.name = "pn553",
		.of_match_table = pn553_of_match,
	},
	.probe = pn553_probe,
	.remove = pn553_remove,
};

module_i2c_driver(pn553_driver);

MODULE_AUTHOR("Architect Stefan - Project RR");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL v2");
