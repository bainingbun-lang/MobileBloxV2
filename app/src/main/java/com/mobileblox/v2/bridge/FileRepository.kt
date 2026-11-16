package com.mobileblox.v2.bridge

import java.io.File

/**
 * Sandboxed file access for the bridge. All paths are resolved below [root],
 * so a bridge client can never read or write outside the app-private folder.
 */
class FileRepository(
    private val root: File,
    private val maxBytes: Int = 5 * 1024 * 1024,
) {
    init {
        root.mkdirs()
    }

    fun read(path: String): String {
        val file = resolve(path)
        require(file.isFile) { "not a file: $path" }
        require(file.length() <= maxBytes) { "file too large: $path" }
        return file.readText()
    }

    fun write(path: String, content: String) {
        val bytes = content.toByteArray(Charsets.UTF_8)
        require(bytes.size <= maxBytes) { "content too large" }
        val file = resolve(path)
        file.parentFile?.mkdirs()
        file.writeText(content)
    }

    fun list(path: String): List<String> {
        val dir = resolve(path)
        require(dir.isDirectory) { "not a directory: $path" }
        return dir.listFiles()
            ?.sortedBy { it.name }
            ?.map { if (it.isDirectory) "${it.name}/" else it.name }
            .orEmpty()
    }

    fun exists(path: String): Boolean = runCatching { resolve(path).exists() }.getOrDefault(false)

    fun rootPath(): String = root.absolutePath

    private fun resolve(path: String): File {
        val rootCanonical = root.canonicalFile
        val cleaned = path.trim().removePrefix("/").replace('\\', '/')
        val candidate = File(rootCanonical, cleaned).canonicalFile
        val inside = candidate.path == rootCanonical.path ||
            candidate.path.startsWith(rootCanonical.path + File.separator)
        require(inside) { "path outside sandbox: $path" }
        return candidate
    }
}
