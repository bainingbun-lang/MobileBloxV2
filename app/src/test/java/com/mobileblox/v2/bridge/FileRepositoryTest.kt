package com.mobileblox.v2.bridge

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Rule
import org.junit.Test
import org.junit.rules.TemporaryFolder

class FileRepositoryTest {

    @get:Rule
    val temp = TemporaryFolder()

    @Test
    fun writesReadsAndListsFiles() {
        val repo = FileRepository(temp.root)
        repo.write("scripts/main.lua", "print('hello')")

        assertEquals("print('hello')", repo.read("scripts/main.lua"))
        assertTrue(repo.list("scripts").contains("main.lua"))
        assertTrue(repo.exists("scripts/main.lua"))
        assertFalse(repo.exists("scripts/missing.lua"))
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsPathTraversalOnRead() {
        FileRepository(temp.root).read("../escape.lua")
    }

    @Test(expected = IllegalArgumentException::class)
    fun rejectsPathTraversalOnWrite() {
        FileRepository(temp.root).write("../escape.lua", "bad")
    }
}
