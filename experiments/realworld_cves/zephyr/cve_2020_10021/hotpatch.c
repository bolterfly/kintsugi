ADDRESS(curr_lba);
ADDRESS(curr_offset);
ADDRESS(block_count);

HOTPATCH_FUNCTION_START_NAKED(infoTransfer, 0x1E, redirect, 0xA) {
	STORE_REGISTERS("r2 - r6");
	VAR_REG(n, "r4", uint32_t);
	VAR_REG(val, "r2", uint32_t);
	VAR_PTR(curr_lba, uint32_t *);
	VAR_PTR(curr_offset, uint16_t *);
	VAR_PTR(block_count, uint32_t *);

	if (n >= block_count) {
		fail();
		sendCSW();
		SET_RETURN_VALUE(0);
		RESTORE_REGISTERS("r2 - r6");
		RESTORE_REGISTERS("r4, r5, r6, pc");
	}

	val = 0;
	*curr_lba = n;
	*curr_offset = val;
	RESTORE_REGISTERS("r2 - r6");
	val = 0;
	HOTPATCH_BRANCH_TO_ORIGINAL_CODE();
}
