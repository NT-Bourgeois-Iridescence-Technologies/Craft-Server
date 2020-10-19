//Common imports
const std = @import("std");
const log = @import("log");
const network = @import("network");
const client = @import("client.zig");

const chat = @import("chat.zig");
const version = @import("version.zig");

pub const PlayerCount = struct{
    online: u32,
    max: u32
};

//Information about the server
pub const ServerInfo = struct{
    description: chat.Text,
    players: PlayerCount,
    version: version.Version
};

pub var info = ServerInfo{
    .description = chat.Text{
        .text = "Hello world",
        .color = "gold"
    },
    .players = PlayerCount{.online = 0, .max = 100},
    .version = version.serverVersion,
};

//Should be running
pub var shouldRun : bool = false;

//Set Evented IO
pub const io_mode = .evented;

//Create our server socket
pub var sock : network.Socket = undefined;


const world = @import("world.zig");
var thread: ?*std.Thread = null;

//Open the network up
//Bind the socket
//Listen for connections
pub fn init() !void {
    shouldRun = true;
    
    //World load before thread spawn
    try world.init();
    thread = try std.Thread.spawn({}, world.tickUpdate);

    try network.init();
    sock = try network.Socket.create(.ipv4, .tcp);
    
    try sock.bindToPort(25565);
    try sock.listen();
    log.info("Listening at {}", .{try sock.getLocalEndPoint()});
}

//Close the socket
//Deinit the network
pub fn deinit() void{
    sock.close();
    network.deinit();
}

//Update connection loop
pub fn update() !void {
    log.info("Waiting for connection", .{});
    
    //TODO: Replace with real allocator
    //Allocate a client
    const cl = try std.heap.page_allocator.create(client.Client);
    defer std.heap.page_allocator.destroy(cl);
    //Destroy the client when done

    //Create a client object
    cl.* = client.Client{
        .conn = try sock.accept(),
        .handle_frame = await async client.Client.handle(cl),
        .status = client.ConnectionStatus.Handshake,
        .protocolVer = 0,
        .compress = false,
        .shouldClose = false,
    };

}

