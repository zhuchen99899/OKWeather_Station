/* include ------------------------------------------------------------------ */
#include <stdbool.h>
#include <string.h>
#include "simu_driver_pin.h"
#include "edf_simu_config.h"
#include "../../../elib/hash_table.h"
#include "../../../edf/normal/elab_pin.h"
#include "../../../common/elab_assert.h"
#include "../../../common/elab_common.h"
#include "../../../common/elab_log.h"

ELAB_TAG("Simu_Pin");

/* private typedef ---------------------------------------------------------- */
typedef struct simu_pin
{
    bool status;
    uint8_t mode;
    
    osMutexId_t mutex;
    elab_pin_t pin;
} simu_pin_t;

/* private function prototype ----------------------------------------------- */
static int32_t _get_pin_from_name(char *name);
static bool _pin_name_valid(const char *name);
static void _init(elab_pin_t * const me);
static void _set_mode(elab_pin_t *me, uint8_t mode);
static bool _get_status(elab_pin_t * const me);
static void _set_status(elab_pin_t * const me, bool status);

/* public variables --------------------------------------------------------- */
extern hash_table_t *ht_simu;

/* private variables -------------------------------------------------------- */
static const elab_pin_ops_t pin_ops =
{
    .init = _init,
    .set_mode = _set_mode,
    .set_status = _set_status,
    .get_status = _get_status,
};

static const osMutexAttr_t mutex_attr_simu_pin =
{
    "MutexSimuPin",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};

/* public function ---------------------------------------------------------- */
void simu_io_new(const char *name, bool is_out)
{
    /* Newly create one hash-table for the simulated PIN. */
    if (ht_simu == NULL)
    {
        ht_simu = hash_table_new(SIMU_HASH_TABLE_SIZE);
        elab_assert(ht_simu != NULL);
    }

    simu_pin_t *simu_pin = elab_malloc(sizeof(simu_pin_t));
    elab_assert(simu_pin != NULL);

    /* Initialize the simulated PIN object. */
    simu_pin->mutex = osMutexNew(&mutex_attr_simu_pin);
    elab_assert(simu_pin->mutex != NULL);
    hash_table_add(ht_simu, (char *)name, (void *)simu_pin);

    /* Register the simulated PIN to edf. */
    elab_pin_register(&simu_pin->pin, name, &pin_ops, simu_pin);
}

void simu_in_set_status(const char *name, bool status)
{
    osStatus_t ret = osOK;

    simu_pin_t *simu_pin = hash_table_get(ht_simu, (char *)name);
    ret = osMutexAcquire(simu_pin->mutex, osWaitForever);
    elab_assert(ret == osOK);

    elab_assert(simu_pin->mode == PIN_MODE_INPUT ||
                simu_pin->mode == PIN_MODE_INPUT_PULLUP ||
                simu_pin->mode == PIN_MODE_INPUT_PULLDOWN);
    simu_pin->status = status;

    ret = osMutexRelease(simu_pin->mutex);
    elab_assert(ret == osOK);
}

bool simu_out_get_status(const char *name)
{
    osStatus_t ret = osOK;

    simu_pin_t *simu_pin = hash_table_get(ht_simu, (char *)name);
    ret = osMutexAcquire(simu_pin->mutex, osWaitForever);
    elab_assert(ret == osOK);

    elab_assert(simu_pin->mode == PIN_MODE_OUTPUT_PP ||
                simu_pin->mode == PIN_MODE_OUTPUT_OD);
    bool _status = simu_pin->status;

    ret = osMutexRelease(simu_pin->mutex);
    elab_assert(ret == osOK);

    return _status;
}

/* private functions -------------------------------------------------------- */
static void _init(elab_pin_t * const me)
{
    (void)me;
}

static void _set_mode(elab_pin_t *me, uint8_t mode)
{
    osStatus_t ret = osOK;

    simu_pin_t *simu_pin =
        (simu_pin_t *)hash_table_get(ht_simu, (char *)me->super.attr.name);

    ret = osMutexAcquire(simu_pin->mutex, osWaitForever);
    elab_assert(ret == osOK);
    simu_pin->mode = mode;
    ret = osMutexRelease(simu_pin->mutex);
    elab_assert(ret == osOK);
}

static void _set_status(elab_pin_t * const me, bool status)
{
    osStatus_t ret = osOK;

    simu_pin_t *simu_pin =
        (simu_pin_t *)hash_table_get(ht_simu, (char *)me->super.attr.name);
    simu_pin->status = status;

    ret = osMutexAcquire(simu_pin->mutex, osWaitForever);
    elab_assert(ret == osOK);
    simu_pin->status = status;
    ret = osMutexRelease(simu_pin->mutex);
    elab_assert(ret == osOK);
}

static bool _get_status(elab_pin_t * const me)
{
    osStatus_t ret = osOK;
    bool status = false;

    simu_pin_t *simu_pin =
        (simu_pin_t *)hash_table_get(ht_simu, (char *)me->super.attr.name);

    ret = osMutexAcquire(simu_pin->mutex, osWaitForever);
    elab_assert(ret == osOK);
    status = simu_pin->status;
    ret = osMutexRelease(simu_pin->mutex);
    elab_assert(ret == osOK);

    return status;
}

static int32_t _get_pin_from_name(char *name)
{
    /* For example, P3.4 P2.18 */
    elab_assert(_pin_name_valid(name));

    return (int32_t)hash_table_index(ht_simu, name);
}

static bool _pin_name_valid(const char *name)
{
    bool ret = true;
    uint8_t len = strlen(name);
    if (len > 5 || len < 4)
    {
        ret = false;
    }
    else if (name[0] != 'P' || name[2] != '.')
    {
        ret = false;
    }
    else if (!(name[1] >= '0' && name[1] <= '9'))
    {
        ret = false;
    }
    else if (!(name[len - 1] >= '0' && name[len - 1] <= '9'))
    {
        ret = false;
    }
    else if (len == 5)
    {
        if (!(name[len - 2] >= '0' && name[len - 2] <= '9' && len == 5))
        {
            ret = false;
        }
    }

    if (!ret)
    {
        elog_error("PIN name %s is invalid.", name);
    }

    return ret;
}

/* ----------------------------- end of file -------------------------------- */
