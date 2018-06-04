/*
 * Copyright 2018, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <cspace/cspace.h>

void test_bf_bit(unsigned long bit)
{
    ZF_LOGV("%lu", bit);
    seL4_Word bitfield[2] = {0};
    assert(bf_first_free(2, bitfield) == 0);

    bf_set_bit(bitfield, bit);
    assert(bf_get_bit(bitfield, bit));
    seL4_Word ff = bit == 0 ? 1 : 0;
    assert(bf_first_free(2, bitfield) == ff);

    bf_clr_bit(bitfield, bit);
    assert(!bf_get_bit(bitfield, bit));
}

static void test_bf(void)
{
    test_bf_bit(0);

    test_bf_bit(1);
    test_bf_bit(63);
    test_bf_bit(64);
    test_bf_bit(65);
    test_bf_bit(127);

    seL4_Word bitfield[2] = {0};
    for (unsigned int i = 0; i < 127; i++) {
        assert(bf_get_bit(bitfield, i) == 0);
        bf_set_bit(bitfield, i);
        assert(bf_get_bit(bitfield, i));
        assert(bf_first_free(2, bitfield) == i+1);
    }
}

static void test_cspace(cspace_t *cspace)
{
    ZF_LOGI("Test cspace");
   /* test we can alloc a cptr */
   ZF_LOGV("Test allocating cslot");
   seL4_CPtr cptr = cspace_alloc_slot(cspace);
   assert(cptr != 0);

   ZF_LOGV("Test freeing cslot");
   /* test we can free the cptr */
   cspace_free_slot(cspace, cptr);

   ZF_LOGV("Test free slot is returned");
   /* test we get the same cptr back if we alloc again */
   seL4_CPtr cptr_new = cspace_alloc_slot(cspace);
   assert(cptr == cptr_new);

   cspace_free_slot(cspace, cptr_new);

   /* test allocating and freeing a large amount of slots */
   int nslots = CNODE_SLOTS(CNODE_SIZE_BITS) / 2;
   if (cspace->two_level) {
       nslots = MIN(CNODE_SLOTS(cspace->top_lvl_size_bits) * CNODE_SLOTS(CNODE_SIZE_BITS) - 4,
                    CNODE_SLOTS(CNODE_SIZE_BITS) * BOT_LVL_PER_NODE + 1);
   }
   seL4_CPtr *slots = malloc(sizeof(seL4_CPtr) * nslots);
   assert(slots != NULL);

   ZF_LOGV("Test allocating and freeing %d slots", nslots);

   for (int i = 0; i < nslots; i++) {
       slots[i] = cspace_alloc_slot(cspace);
       assert(i == 0 || slots[i-1] == slots[i] - 1);
       assert(slots[i] != seL4_CapNull);
   }

   ZF_LOGV("Allocated %lu <-> %lu slots\n", slots[0], slots[nslots - 1]);

   for (int i = 0; i < nslots; i++) {
       cspace_free_slot(cspace, slots[i]);
   }

   free(slots);
}

void run_tests(cspace_t *cspace)
{
    /* test the cspace bitfield data structure */
    test_bf();

    /* test the root cspace */
    test_cspace(cspace);
    ZF_LOGI("Root CSpace test passed!");

    /* test a new, 1-level cspace */
    cspace_t dummy_cspace;
    int error = cspace_create_one_level(cspace, &dummy_cspace);
    assert(error == 0);
    test_cspace(&dummy_cspace);
    cspace_destroy(&dummy_cspace);
    ZF_LOGI("Single level cspace test passed!");

    /* test a new, 2 level cspace */
    cspace_alloc_t cspace_alloc = {
        .map_frame = bootstrap_cspace_map_frame,
        .alloc_4k_ut = bootstrap_cspace_alloc_4k_ut,
        .free_4k_ut = bootstrap_cspace_free_4k_ut,
        .cookie = NULL
    };
    error = cspace_create_two_level(cspace, &dummy_cspace, cspace_alloc);
    assert(error == 0);
    test_cspace(&dummy_cspace);
    cspace_destroy(&dummy_cspace);
    ZF_LOGI("Double level cspace test passed!");
}