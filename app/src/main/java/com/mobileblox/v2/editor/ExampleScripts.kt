package com.mobileblox.v2.editor

/** Ready-to-run snippets for the simulated Roblox API. */
object ExampleScripts {

    val all: List<Pair<String, String>> = listOf(
        "Hola mundo" to "print(\"Hola desde MobileBlox\")",
        "Bucle" to "for i = 1, 10 do\n    print(\"i =\", i)\nend",
        "Instancia" to "local part = Instance.new(\"Part\")\n" +
            "part.Name = \"MiParte\"\n" +
            "part.Parent = workspace\n" +
            "print(part, part.Parent)",
        "Jugador" to "local player = game:GetService(\"Players\").LocalPlayer\n" +
            "print(player.Name, player.UserId)",
        "Vector3" to "local v = Vector3.new(1, 2, 3)\n" +
            "print(v, v.Magnitude, v.Unit)",
        "Humanoid" to "local humanoid = workspace:FindFirstChild(\"Player\"):FindFirstChild(\"Humanoid\")\n" +
            "print(\"Vida:\", humanoid.Health)\n" +
            "humanoid:TakeDamage(10)\n" +
            "print(\"Vida:\", humanoid.Health)",
    )
}
