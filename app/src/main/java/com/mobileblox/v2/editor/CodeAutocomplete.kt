package com.mobileblox.v2.editor

import android.content.Context
import android.widget.ArrayAdapter
import android.widget.MultiAutoCompleteTextView

/** Lightweight autocomplete for the Luau editor. */
object CodeAutocomplete {

    private val WORDS = arrayOf(
        "print", "warn", "local", "function", "end", "if", "then", "else", "elseif",
        "for", "while", "do", "return", "break", "continue", "in", "and", "or", "not",
        "nil", "true", "false",
        "game", "Game", "workspace", "Workspace", "Players", "script", "Instance",
        "Instance.new", "GetService", "FindFirstChild", "FindFirstChildOfClass",
        "GetChildren", "GetDescendants", "WaitForChild", "Destroy", "Clone", "IsA",
        "Parent", "Name", "ClassName", "Position", "CFrame", "Size", "Color",
        "Transparency", "Anchored", "CanCollide", "Health", "MaxHealth", "WalkSpeed",
        "JumpPower", "Humanoid", "HumanoidRootPart", "Character", "LocalPlayer",
        "UserId", "DisplayName", "PlayerAdded", "PlayerRemoving",
        "Vector3.new", "CFrame.new", "Color3.new", "Color3.fromRGB", "UDim2.new",
        "UDim.new", "Random.new", "Enum", "typeof", "getgenv", "getrenv", "loadstring",
        "task.wait", "task.spawn", "task.delay", "task.defer", "task.cancel",
        "wait", "spawn", "delay", "pcall", "tostring", "tonumber", "pairs", "ipairs",
        "table.insert", "table.remove", "table.concat", "string.format",
        "math.random", "math.floor", "math.ceil", "math.max", "math.min",
    )

    fun attach(view: MultiAutoCompleteTextView, context: Context) {
        val adapter = ArrayAdapter(context, android.R.layout.simple_dropdown_item_1line, WORDS)
        view.setAdapter(adapter)
        view.setThreshold(1)
        view.setTokenizer(Tokenizer())
    }

    private class Tokenizer : MultiAutoCompleteTextView.Tokenizer {
        override fun findTokenStart(text: CharSequence, cursor: Int): Int {
            var index = cursor
            while (index > 0) {
                val character = text[index - 1]
                if (character.isLetterOrDigit() || character == '_' || character == '.') {
                    index--
                } else {
                    break
                }
            }
            return index
        }

        override fun findTokenEnd(text: CharSequence, cursor: Int): Int {
            var index = cursor
            while (index < text.length) {
                val character = text[index]
                if (character.isLetterOrDigit() || character == '_' || character == '.') {
                    index++
                } else {
                    break
                }
            }
            return index
        }

        override fun terminateToken(text: CharSequence): CharSequence = text
    }
}
