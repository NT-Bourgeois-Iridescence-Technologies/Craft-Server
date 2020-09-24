const std = @import("std");
const log = @import("log");
const network = @import("network");
const client = @import("client.zig");

pub var shouldRun : bool = false;
pub var sock : network.Socket = undefined;

pub const io_mode = .evented;

pub fn init() !void {
    shouldRun = true;
    
    try network.init();
    
    sock = try network.Socket.create(.ipv4, .tcp);
    
    try sock.bindToPort(25565);
    try sock.listen();
    log.info("Listening at {}", .{try sock.getLocalEndPoint()});
}

pub fn deinit() void{
    network.deinit();
    sock.close();
}

pub fn update() !void {
    log.info("Waiting for connection", .{});
        
    const cl = try std.heap.page_allocator.create(client.Client);
    cl.* = client.Client{
        .conn = try sock.accept(),
        .handle_frame = async client.Client.handle(cl),
    };
    
}

