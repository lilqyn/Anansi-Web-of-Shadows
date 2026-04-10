// ============================================================
// ANANSI: WEB OF SHADOWS — Action-Adventure Prototype
// Complete game engine — single file, no external assets
// ============================================================

"use strict";

// ============ CONSTANTS ============
const WORLD_W = 8000;
const WORLD_H = 900;
const GRAVITY = 0.65;
const TILE = 40;
const CAM_SMOOTH = 0.08;
const CAM_LEAD = 60;

// Area boundaries (x positions)
const AREAS = {
    festival:     { x: 0,    w: 1400, name: "The Festival Grounds",  sub: "A night of celebration" },
    cityEscape:   { x: 1400, w: 1400, name: "City Streets",          sub: "Chaos in the capital" },
    shrine:       { x: 2800, w: 800,  name: "Anansi's Shrine",       sub: "Where threads converge" },
    corrupted:    { x: 3600, w: 1600, name: "The Corrupted Quarter",  sub: "Between worlds" },
    proverbGate:  { x: 5200, w: 800,  name: "The Proverb Gate",      sub: "Wisdom opens the way" },
    bossArena:    { x: 6000, w: 1200, name: "The Mask Chamber",      sub: "Face the Guardian" },
    ending:       { x: 7200, w: 800,  name: "",                       sub: "" }
};

// Colors
const COL = {
    gold: '#d4af37', goldBright: '#ffd700', goldDark: '#b8860b',
    skinWarm: '#8B6914', skinDark: '#6B4E12',
    cloth1: '#D4682A', cloth2: '#2A5C8B', cloth3: '#8B2A5C',
    festivalOrange: '#E87530', festivalRed: '#C0392B', festivalGold: '#F4D03F',
    stone: '#4A4A52', stoneLight: '#6B6B75', stoneDark: '#2C2C34',
    wood: '#6B4226', woodLight: '#8B5A2B', woodDark: '#3E2723',
    sky: '#0B0B2E', skyCorrupted: '#1A0A2E',
    spiritBlue: '#5DADE2', spiritSilver: '#AAB7C4',
    corruption: '#7B2FBE', corruptionLight: '#A855F7',
    fire: '#FF6B2B', fireBright: '#FFD700',
    green: '#27AE60', red: '#E74C3C', yellow: '#F1C40F'
};

// ============ GLOBAL STATE ============
const Game = {
    canvas: null, ctx: null,
    width: 0, height: 0,
    running: false,
    time: 0, dt: 0, lastTime: 0,
    state: 'title', // title, playing, dialogue, puzzle, cutscene, paused, demo_complete
    slowMo: 1, slowMoTimer: 0,
    screenShake: 0, shakeIntensity: 0,
    frameCount: 0,

    // Stats
    stats: { enemies: 0, fragments: 0, stealth: 0, maxCombo: 0 },

    start() {
        document.getElementById('title-screen').style.display = 'none';
        document.getElementById('hud').style.display = 'block';
        this.state = 'playing';
        this.running = true;
        this.init();
    },

    restart() {
        location.reload();
    },

    init() {
        this.canvas = document.getElementById('gameCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.resize();
        window.addEventListener('resize', () => this.resize());

        Input.init();
        Camera.init();
        World.init();
        Player.init();
        NPCManager.init();
        EnemyManager.init();
        ParticleSystem.init();
        Audio.init();
        Dialogue.init();
        Puzzle.init();
        Cinematic.init();

        // Start with festival cinematic
        Cinematic.startFestival();

        this.lastTime = performance.now();
        this.loop();
    },

    resize() {
        this.width = window.innerWidth;
        this.height = window.innerHeight;
        this.canvas.width = this.width;
        this.canvas.height = this.height;
    },

    loop() {
        const now = performance.now();
        let rawDt = (now - this.lastTime) / 1000;
        this.lastTime = now;
        rawDt = Math.min(rawDt, 0.05);
        this.dt = rawDt * this.slowMo;
        this.time += this.dt;
        this.frameCount++;

        if (this.slowMoTimer > 0) {
            this.slowMoTimer -= rawDt;
            if (this.slowMoTimer <= 0) { this.slowMo = 1; this.slowMoTimer = 0; }
        }

        if (this.screenShake > 0) this.screenShake -= rawDt * 8;

        if (this.running) {
            this.update();
            this.render();
        }

        requestAnimationFrame(() => this.loop());
    },

    update() {
        if (this.state === 'playing' || this.state === 'cutscene') {
            Player.update(this.dt);
            Camera.update(this.dt);
            NPCManager.update(this.dt);
            EnemyManager.update(this.dt);
            ParticleSystem.update(this.dt);
            Objective.update(this.dt);
            World.update(this.dt);
            Audio.update(this.dt);
            Cinematic.update(this.dt);
        } else if (this.state === 'dialogue') {
            Dialogue.update(this.dt);
            ParticleSystem.update(this.dt);
            Camera.update(this.dt);
        } else if (this.state === 'puzzle') {
            Puzzle.update(this.dt);
            Player.update(this.dt);
            Camera.update(this.dt);
            EnemyManager.update(this.dt);
            ParticleSystem.update(this.dt);
        }
        HUD.update(this.dt);
        Input.endFrame();
    },

    render() {
        const ctx = this.ctx;
        ctx.save();

        // Screen shake
        if (this.screenShake > 0) {
            const sx = (Math.random() - 0.5) * this.screenShake * this.shakeIntensity;
            const sy = (Math.random() - 0.5) * this.screenShake * this.shakeIntensity;
            ctx.translate(sx, sy);
        }

        // Clear
        ctx.fillStyle = COL.sky;
        ctx.fillRect(0, 0, this.width, this.height);

        // Parallax backgrounds
        World.renderBackground(ctx);

        // Camera transform
        ctx.save();
        ctx.translate(-Camera.x, -Camera.y);

        // World geometry
        World.render(ctx);

        // NPCs
        NPCManager.render(ctx);

        // Enemies
        EnemyManager.render(ctx);

        // Boss
        BossManager.render(ctx);

        // Player
        Player.render(ctx);

        // Particles (world space)
        ParticleSystem.render(ctx);

        // Puzzle elements
        if (Puzzle.started) {
            Puzzle.render(ctx);
        }

        ctx.restore(); // camera

        // Screen-space effects
        Cinematic.render(ctx);

        // Spider Sense overlay
        if (Player.spiderSenseActive) {
            ctx.fillStyle = 'rgba(212,175,55,0.04)';
            ctx.fillRect(0, 0, this.width, this.height);
        }

        ctx.restore(); // shake
    },

    triggerSlowMo(factor, duration) {
        this.slowMo = factor;
        this.slowMoTimer = duration;
    },

    shake(intensity, amount) {
        this.screenShake = Math.max(this.screenShake, amount);
        this.shakeIntensity = intensity;
    }
};

// ============ INPUT ============
const Input = {
    keys: {},
    justPressed: {},

    init() {
        window.addEventListener('keydown', e => {
            if (!this.keys[e.code]) this.justPressed[e.code] = true;
            this.keys[e.code] = true;
            if (['Space','ArrowUp','ArrowDown','ArrowLeft','ArrowRight','KeyW','KeyA','KeyS','KeyD','KeyQ','KeyE','KeyR','KeyF','KeyV','ShiftLeft','ShiftRight'].includes(e.code)) {
                e.preventDefault();
            }
        });
        window.addEventListener('keyup', e => {
            this.keys[e.code] = false;
        });
        // Mouse click also triggers attack
        window.addEventListener('mousedown', e => {
            if (e.button === 0) {
                if (!this.keys['Space']) this.justPressed['Space'] = true;
                this.keys['Space'] = true;
            }
        });
        window.addEventListener('mouseup', e => {
            if (e.button === 0) this.keys['Space'] = false;
        });
    },

    isDown(code) { return !!this.keys[code]; },
    wasPressed(code) { return !!this.justPressed[code]; },

    endFrame() {
        this.justPressed = {};
    },

    get left() { return this.isDown('KeyA') || this.isDown('ArrowLeft'); },
    get right() { return this.isDown('KeyD') || this.isDown('ArrowRight'); },
    get up() { return this.isDown('KeyW') || this.isDown('ArrowUp'); },
    get down() { return this.isDown('KeyS') || this.isDown('ArrowDown'); },
    get jump() { return this.wasPressed('KeyW') || this.wasPressed('ArrowUp'); },
    get crouch() { return this.isDown('KeyS') || this.isDown('ArrowDown'); },
    get attack() { return this.wasPressed('Space'); },
    get dodge() { return this.wasPressed('ShiftLeft') || this.wasPressed('ShiftRight'); },
    get parry() { return this.wasPressed('KeyQ'); },
    get silkenPath() { return this.wasPressed('KeyE'); },
    get spiderSense() { return this.wasPressed('KeyR'); },
    get trickMirror() { return this.wasPressed('KeyF'); },
    get borrowedVoice() { return this.wasPressed('KeyV'); }
};

// ============ CAMERA ============
const Camera = {
    x: 0, y: 0,
    targetX: 0, targetY: 0,

    init() {
        this.x = 0;
        this.y = 0;
    },

    update(dt) {
        const pw = Game.width;
        const ph = Game.height;

        this.targetX = Player.x + Player.facing * CAM_LEAD - pw / 2;
        this.targetY = Player.y - ph / 2 + 100;

        // Clamp
        this.targetX = Math.max(0, Math.min(WORLD_W - pw, this.targetX));
        this.targetY = Math.max(-200, Math.min(WORLD_H - ph + 100, this.targetY));

        this.x += (this.targetX - this.x) * CAM_SMOOTH;
        this.y += (this.targetY - this.y) * CAM_SMOOTH;
    },

    worldToScreen(wx, wy) {
        return { x: wx - this.x, y: wy - this.y };
    }
};

// ============ WORLD ============
const World = {
    platforms: [],
    walls: [],
    decorations: [],
    latchPoints: [],
    climbSurfaces: [],
    hideSpots: [],
    spiritBreached: false,
    breachTimer: 0,
    festivalLights: [],
    corruptionParticleTimer: 0,

    init() {
        this.buildFestival();
        this.buildCityEscape();
        this.buildShrine();
        this.buildCorruptedQuarter();
        this.buildProverbGate();
        this.buildBossArena();
        this.buildEnding();
        this.generateFestivalLights();
    },

    update(dt) {
        if (this.spiritBreached) {
            this.breachTimer += dt;
            this.corruptionParticleTimer += dt;
            if (this.corruptionParticleTimer > 0.15) {
                const area = this.getArea(Player.x);
                if (area === 'corrupted' || area === 'bossArena' || area === 'proverbGate') {
                    this.corruptionParticleTimer = 0;
                    const cx = Player.x + (Math.random() - 0.5) * Game.width * 0.8;
                    const cy = Camera.y + Math.random() * Game.height;
                    ParticleSystem.emit(cx, cy, 1, {
                        color: COL.corruptionLight, life: 2.5, size: 3, speed: 15,
                        gravity: -0.05, fadeOut: true
                    });
                }
            }
        }
    },

    addPlatform(x, y, w, h, type) {
        this.platforms.push({ x, y, w, h: h || TILE, type: type || 'stone' });
    },

    addWall(x, y, w, h) {
        this.walls.push({ x, y, w, h });
    },

    buildFestival() {
        const a = AREAS.festival;
        // Ground
        this.addPlatform(a.x, 700, a.w + 200, 200, 'ground');
        // Market stalls (as platforms)
        this.addPlatform(a.x + 200, 620, 120, 15, 'wood');
        this.addPlatform(a.x + 500, 600, 140, 15, 'wood');
        this.addPlatform(a.x + 850, 620, 100, 15, 'wood');
        // Stage
        this.addPlatform(a.x + 1050, 640, 250, 15, 'wood');
        // Hide spots
        this.hideSpots.push(
            { x: a.x + 210, y: 625, w: 100, h: 75 },
            { x: a.x + 510, y: 605, w: 120, h: 95 }
        );
        // Decorations
        this.decorations.push(
            { type: 'banner', x: a.x + 100, y: 500, color: COL.festivalRed },
            { type: 'banner', x: a.x + 350, y: 480, color: COL.festivalGold },
            { type: 'banner', x: a.x + 600, y: 490, color: COL.festivalOrange },
            { type: 'banner', x: a.x + 900, y: 500, color: COL.festivalRed },
            { type: 'lantern', x: a.x + 180, y: 460 },
            { type: 'lantern', x: a.x + 420, y: 440 },
            { type: 'lantern', x: a.x + 680, y: 450 },
            { type: 'lantern', x: a.x + 950, y: 460 },
            { type: 'lantern', x: a.x + 1150, y: 430 },
            { type: 'drum', x: a.x + 1000, y: 680 },
            { type: 'drum', x: a.x + 1270, y: 680 },
            { type: 'stall', x: a.x + 200, y: 570, w: 120 },
            { type: 'stall', x: a.x + 500, y: 550, w: 140 },
            { type: 'stall', x: a.x + 850, y: 570, w: 100 }
        );
    },

    buildCityEscape() {
        const a = AREAS.cityEscape;
        // Ground
        this.addPlatform(a.x - 100, 700, a.w + 200, 200, 'ground');

        // Buildings — walls only in upper portion, ground level stays open for walking
        // Each building has a solid upper wall + rooftop, but a passage gap at ground level
        this.addWall(a.x + 100, 350, 80, 250);   // stops at y=600, gap below
        this.addWall(a.x + 400, 300, 100, 280);   // stops at y=580
        this.addWall(a.x + 700, 380, 80, 220);    // stops at y=600
        this.addWall(a.x + 1000, 280, 120, 300);  // stops at y=580

        // Rooftop platforms (on top of buildings)
        this.addPlatform(a.x + 80, 350, 120, 15, 'stone');
        this.addPlatform(a.x + 380, 300, 140, 15, 'stone');
        this.addPlatform(a.x + 680, 380, 100, 15, 'stone');
        this.addPlatform(a.x + 980, 280, 160, 15, 'stone');

        // Mid-height stepping platforms (parkour route)
        this.addPlatform(a.x + 220, 540, 80, 15, 'wood');
        this.addPlatform(a.x + 280, 440, 80, 15, 'wood');
        this.addPlatform(a.x + 500, 520, 100, 15, 'wood');
        this.addPlatform(a.x + 580, 400, 80, 15, 'wood');
        this.addPlatform(a.x + 800, 540, 80, 15, 'wood');
        this.addPlatform(a.x + 850, 340, 80, 15, 'wood');
        this.addPlatform(a.x + 1200, 360, 100, 15, 'stone');

        // Climb surfaces — on the LEFT (approach) side of each building
        this.climbSurfaces.push(
            { x: a.x + 100, y: 350, h: 250 },
            { x: a.x + 400, y: 300, h: 280 },
            { x: a.x + 700, y: 380, h: 220 },
            { x: a.x + 1000, y: 280, h: 300 }
        );

        // Latch points (above buildings for web-swing)
        this.latchPoints.push(
            { x: a.x + 250, y: 280 },
            { x: a.x + 550, y: 260 },
            { x: a.x + 900, y: 240 },
            { x: a.x + 1150, y: 280 }
        );

        // Hide spots (behind crates/stalls at ground level)
        this.hideSpots.push(
            { x: a.x + 300, y: 610, w: 80, h: 90 },
            { x: a.x + 600, y: 620, w: 60, h: 80 },
            { x: a.x + 900, y: 610, w: 80, h: 90 }
        );

        // Decorations
        this.decorations.push(
            { type: 'torch', x: a.x + 160, y: 600 },
            { type: 'torch', x: a.x + 460, y: 600 },
            { type: 'torch', x: a.x + 760, y: 600 },
            { type: 'torch', x: a.x + 1060, y: 600 },
            { type: 'building_face', x: a.x + 100, y: 350, w: 80, h: 350 },
            { type: 'building_face', x: a.x + 400, y: 300, w: 100, h: 400 },
            { type: 'building_face', x: a.x + 700, y: 380, w: 80, h: 320 },
            { type: 'building_face', x: a.x + 1000, y: 280, w: 120, h: 420 }
        );
    },

    buildShrine() {
        const a = AREAS.shrine;
        this.addPlatform(a.x - 100, 700, a.w + 200, 200, 'ground');
        // Walls — with doorway gaps at ground level for entry/exit
        this.addWall(a.x + 50, 250, 40, 350);    // left wall stops at y=600, leaves gap
        this.addWall(a.x + a.w - 90, 250, 40, 350); // right wall same
        // Ceiling
        this.addPlatform(a.x + 50, 250, a.w - 140, 20, 'stone');
        // Inner step
        this.addPlatform(a.x + 200, 580, 120, 15, 'stone');
        this.addPlatform(a.x + 480, 580, 120, 15, 'stone');
        // Decorations
        this.decorations.push(
            { type: 'adinkra_wall', x: a.x + 200, y: 380, symbol: 'gye_nyame' },
            { type: 'adinkra_wall', x: a.x + 400, y: 350, symbol: 'sankofa' },
            { type: 'adinkra_wall', x: a.x + 600, y: 380, symbol: 'dwennimmen' },
            { type: 'shrine_altar', x: a.x + 400, y: 660 },
            { type: 'web_glow', x: a.x + 400, y: 420 }
        );
    },

    buildCorruptedQuarter() {
        const a = AREAS.corrupted;
        this.addPlatform(a.x - 100, 700, a.w + 200, 200, 'corrupted_ground');
        // Platforms
        this.addPlatform(a.x + 100, 560, 150, 15, 'stone');
        this.addPlatform(a.x + 350, 490, 120, 15, 'stone');
        this.addPlatform(a.x + 550, 530, 100, 15, 'stone');
        this.addPlatform(a.x + 750, 460, 130, 15, 'stone');
        this.addPlatform(a.x + 950, 560, 100, 15, 'stone');
        this.addPlatform(a.x + 1100, 490, 120, 15, 'stone');
        this.addPlatform(a.x + 1350, 420, 100, 15, 'stone');
        // Walls — leave ground-level gaps for passage
        this.addWall(a.x + 250, 400, 60, 200);
        this.addWall(a.x + 650, 350, 60, 230);
        this.addWall(a.x + 1050, 380, 60, 210);
        // Climb surfaces — on left (approach) side
        this.climbSurfaces.push(
            { x: a.x + 250, y: 400, h: 200 },
            { x: a.x + 650, y: 350, h: 230 },
            { x: a.x + 1050, y: 380, h: 210 }
        );
        // Latch points
        this.latchPoints.push(
            { x: a.x + 400, y: 340 },
            { x: a.x + 800, y: 290 },
            { x: a.x + 1250, y: 320 }
        );
        // Hide spots
        this.hideSpots.push(
            { x: a.x + 450, y: 610, w: 70, h: 90 },
            { x: a.x + 850, y: 610, w: 70, h: 90 },
            { x: a.x + 1250, y: 610, w: 70, h: 90 }
        );
        // Corruption decorations
        this.decorations.push(
            { type: 'spirit_thread', x: a.x + 150, y: 370, w: 200 },
            { type: 'spirit_thread', x: a.x + 500, y: 320, w: 250 },
            { type: 'spirit_thread', x: a.x + 900, y: 340, w: 300 },
            { type: 'spirit_thread', x: a.x + 1200, y: 300, w: 200 },
            { type: 'floating_cloth', x: a.x + 300, y: 420 },
            { type: 'floating_cloth', x: a.x + 700, y: 370 },
            { type: 'floating_cloth', x: a.x + 1100, y: 400 },
            { type: 'cracked_adinkra', x: a.x + 500, y: 520 },
            { type: 'cracked_adinkra', x: a.x + 900, y: 500 }
        );
    },

    buildProverbGate() {
        const a = AREAS.proverbGate;
        this.addPlatform(a.x - 100, 700, a.w + 200, 200, 'ground');
        // Gate wall — blocks passage until puzzle is solved (placed at far right of area)
        this.addWall(a.x + 650, 300, 100, 400);
        this.decorations.push(
            { type: 'proverb_gate', x: a.x + 650, y: 300, w: 100, h: 400 }
        );
    },

    buildBossArena() {
        const a = AREAS.bossArena;
        this.addPlatform(a.x - 50, 700, a.w + 100, 200, 'arena_ground');
        // Elevated
        this.addPlatform(a.x + 100, 480, 100, 15, 'stone');
        this.addPlatform(a.x + 500, 430, 120, 15, 'stone');
        this.addPlatform(a.x + 900, 480, 100, 15, 'stone');
        // Walls — left has entry gap, right blocks escape
        this.addWall(a.x - 20, 200, 30, 380);   // left wall, gap at bottom
        this.addWall(a.x + a.w - 10, 200, 30, 500); // right wall, solid
        // Latch points
        this.latchPoints.push(
            { x: a.x + 300, y: 300 },
            { x: a.x + 600, y: 250 },
            { x: a.x + 900, y: 300 }
        );
        // Fires
        this.decorations.push(
            { type: 'ceremonial_fire', x: a.x + 150, y: 660 },
            { type: 'ceremonial_fire', x: a.x + 450, y: 660 },
            { type: 'ceremonial_fire', x: a.x + 750, y: 660 },
            { type: 'ceremonial_fire', x: a.x + 1050, y: 660 }
        );
    },

    buildEnding() {
        const a = AREAS.ending;
        this.addPlatform(a.x - 100, 700, a.w + 200, 200, 'ground');
    },

    generateFestivalLights() {
        for (let i = 0; i < 30; i++) {
            this.festivalLights.push({
                x: AREAS.festival.x + Math.random() * AREAS.festival.w,
                y: 300 + Math.random() * 300,
                size: 2 + Math.random() * 3,
                speed: 0.5 + Math.random() * 1.5,
                phase: Math.random() * Math.PI * 2
            });
        }
    },

    getArea(x) {
        for (const [key, a] of Object.entries(AREAS)) {
            if (x >= a.x && x < a.x + a.w) return key;
        }
        return 'ending';
    },

    renderBackground(ctx) {
        const pw = Game.width;
        const ph = Game.height;
        const area = this.getArea(Player.x);

        // Sky gradient
        let skyTop, skyBot;
        if (area === 'corrupted' || area === 'bossArena' || area === 'proverbGate') {
            skyTop = '#0A0520'; skyBot = '#1A0A35';
        } else if (area === 'shrine') {
            skyTop = '#050510'; skyBot = '#0A0A20';
        } else if (this.spiritBreached) {
            const t = Math.min(this.breachTimer / 3, 1);
            skyTop = lerpColor('#0B1535', '#120525', t);
            skyBot = lerpColor('#1A2555', '#1A0A35', t);
        } else {
            skyTop = '#0B1535'; skyBot = '#1A2555';
        }

        const grad = ctx.createLinearGradient(0, 0, 0, ph);
        grad.addColorStop(0, skyTop);
        grad.addColorStop(1, skyBot);
        ctx.fillStyle = grad;
        ctx.fillRect(0, 0, pw, ph);

        // Stars
        ctx.fillStyle = 'rgba(255,255,255,0.6)';
        for (let i = 0; i < 80; i++) {
            const sx = ((i * 137 + 42) % 2000) - (Camera.x * 0.02) % 2000;
            const sy = ((i * 97 + 84) % 400);
            const ss = 1 + (i % 3);
            const twinkle = Math.sin(Game.time * 2 + i) * 0.3 + 0.7;
            ctx.globalAlpha = twinkle * 0.6;
            ctx.fillRect(((sx % pw) + pw) % pw, sy, ss, ss);
        }
        ctx.globalAlpha = 1;

        // Moon
        if (area !== 'shrine') {
            const moonX = pw * 0.75 - Camera.x * 0.03;
            ctx.beginPath();
            ctx.arc(moonX, 100, 35, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(255,240,200,0.9)';
            ctx.fill();
            ctx.beginPath();
            ctx.arc(moonX + 10, 95, 30, 0, Math.PI * 2);
            ctx.fillStyle = skyTop;
            ctx.fill();
        }

        // Mountains (parallax)
        ctx.fillStyle = 'rgba(20,15,40,0.8)';
        ctx.beginPath();
        ctx.moveTo(0, ph);
        for (let x = 0; x <= pw; x += 60) {
            const wx = x + Camera.x * 0.1;
            const h = Math.sin(wx * 0.003) * 80 + Math.sin(wx * 0.007) * 40 + 200;
            ctx.lineTo(x, ph - h);
        }
        ctx.lineTo(pw, ph);
        ctx.fill();

        // Treeline
        ctx.fillStyle = 'rgba(15,12,30,0.9)';
        ctx.beginPath();
        ctx.moveTo(0, ph);
        for (let x = 0; x <= pw; x += 30) {
            const wx = x + Camera.x * 0.25;
            const h = Math.sin(wx * 0.008) * 50 + Math.sin(wx * 0.015 + 1) * 30 + 130;
            ctx.lineTo(x, ph - h);
        }
        ctx.lineTo(pw, ph);
        ctx.fill();

        // Festival lights
        if (!this.spiritBreached) {
            for (const fl of this.festivalLights) {
                const sx = fl.x - Camera.x * 0.7;
                const sy = fl.y + Math.sin(Game.time * fl.speed + fl.phase) * 10;
                if (sx > -20 && sx < pw + 20) {
                    const alpha = Math.sin(Game.time * 1.5 + fl.phase) * 0.3 + 0.5;
                    ctx.beginPath();
                    ctx.arc(sx, sy, fl.size, 0, Math.PI * 2);
                    ctx.fillStyle = `rgba(255,200,80,${alpha})`;
                    ctx.fill();
                }
            }
        }

        // Corruption threads in background
        if (this.spiritBreached && (area === 'corrupted' || area === 'bossArena' || area === 'proverbGate')) {
            ctx.strokeStyle = 'rgba(160,80,255,0.06)';
            ctx.lineWidth = 1;
            for (let i = 0; i < 10; i++) {
                const bx = ((i * 300 - Camera.x * 0.15) % pw + pw) % pw;
                const by = 100 + Math.sin(Game.time * 0.5 + i) * 60;
                ctx.beginPath();
                ctx.moveTo(bx, by);
                ctx.bezierCurveTo(bx + 80, by + 40, bx + 160, by - 30, bx + 250, by + 20);
                ctx.stroke();
            }
        }
    },

    render(ctx) {
        const camL = Camera.x - 100;
        const camR = Camera.x + Game.width + 100;

        // Decorations behind
        for (const d of this.decorations) {
            if (d.x < camL - 200 || d.x > camR + 200) continue;
            this.renderDecoration(ctx, d);
        }

        // Platforms
        for (const p of this.platforms) {
            if (p.x + p.w < camL || p.x > camR) continue;
            this.renderPlatform(ctx, p);
        }

        // Walls
        for (const w of this.walls) {
            if (w.x + w.w < camL || w.x > camR) continue;
            this.renderWall(ctx, w);
        }

        // Climb indicators
        for (const cs of this.climbSurfaces) {
            if (cs.x < camL || cs.x > camR) continue;
            ctx.strokeStyle = 'rgba(212,175,55,0.12)';
            ctx.lineWidth = 2;
            ctx.setLineDash([6, 8]);
            ctx.beginPath();
            ctx.moveTo(cs.x, cs.y);
            ctx.lineTo(cs.x, cs.y + cs.h);
            ctx.stroke();
            ctx.setLineDash([]);
            // Small grip marks
            for (let y = cs.y + 20; y < cs.y + cs.h; y += 40) {
                ctx.fillStyle = 'rgba(212,175,55,0.1)';
                ctx.fillRect(cs.x - 3, y, 6, 8);
            }
        }

        // Latch points
        if (Player.abilities.silkenPath) {
            for (const lp of this.latchPoints) {
                if (lp.x < camL || lp.x > camR) continue;
                const pulse = Math.sin(Game.time * 3) * 0.3 + 0.5;
                ctx.beginPath();
                ctx.arc(lp.x, lp.y, 8, 0, Math.PI * 2);
                ctx.fillStyle = `rgba(212,175,55,${pulse * 0.3})`;
                ctx.fill();
                ctx.strokeStyle = `rgba(212,175,55,${pulse * 0.8})`;
                ctx.lineWidth = 1.5;
                ctx.stroke();
                // Web pattern
                for (let a = 0; a < 6; a++) {
                    const angle = (a / 6) * Math.PI * 2 + Game.time * 0.5;
                    ctx.beginPath();
                    ctx.moveTo(lp.x, lp.y);
                    ctx.lineTo(lp.x + Math.cos(angle) * 14, lp.y + Math.sin(angle) * 14);
                    ctx.strokeStyle = `rgba(212,175,55,${pulse * 0.2})`;
                    ctx.lineWidth = 1;
                    ctx.stroke();
                }
            }
        }

        // Hide spots (visible with Spider Sense)
        if (Player.spiderSenseActive) {
            for (const hs of this.hideSpots) {
                if (hs.x < camL || hs.x > camR) continue;
                ctx.strokeStyle = 'rgba(212,175,55,0.35)';
                ctx.lineWidth = 1.5;
                ctx.setLineDash([4, 4]);
                ctx.strokeRect(hs.x, hs.y, hs.w, hs.h);
                ctx.setLineDash([]);
                ctx.fillStyle = 'rgba(212,175,55,0.05)';
                ctx.fillRect(hs.x, hs.y, hs.w, hs.h);
            }
        }
    },

    renderPlatform(ctx, p) {
        if (p.type === 'ground' || p.type === 'corrupted_ground' || p.type === 'arena_ground') {
            const grad = ctx.createLinearGradient(p.x, p.y, p.x, p.y + 60);
            if (p.type === 'corrupted_ground') {
                grad.addColorStop(0, '#1A0A2E');
                grad.addColorStop(1, '#0A0515');
            } else if (p.type === 'arena_ground') {
                grad.addColorStop(0, COL.stone);
                grad.addColorStop(1, COL.stoneDark);
            } else {
                grad.addColorStop(0, '#2C1810');
                grad.addColorStop(1, '#1A0E08');
            }
            ctx.fillStyle = grad;
            ctx.fillRect(p.x, p.y, p.w, p.h);
            // Top edge
            ctx.fillStyle = p.type === 'corrupted_ground' ? 'rgba(120,50,200,0.3)' :
                           p.type === 'arena_ground' ? COL.stoneLight : '#3E2723';
            ctx.fillRect(p.x, p.y, p.w, 3);
            // Surface detail
            ctx.fillStyle = 'rgba(255,255,255,0.02)';
            for (let x = p.x; x < p.x + p.w; x += 15 + ((x * 7) & 15)) {
                ctx.fillRect(x, p.y + 4, 6 + ((x * 3) & 7), 2);
            }
        } else if (p.type === 'stone') {
            ctx.fillStyle = COL.stone;
            ctx.fillRect(p.x, p.y, p.w, p.h);
            ctx.fillStyle = COL.stoneLight;
            ctx.fillRect(p.x, p.y, p.w, 2);
            ctx.fillStyle = 'rgba(0,0,0,0.3)';
            ctx.fillRect(p.x, p.y + p.h - 2, p.w, 2);
        } else if (p.type === 'wood') {
            ctx.fillStyle = COL.wood;
            ctx.fillRect(p.x, p.y, p.w, p.h);
            ctx.fillStyle = COL.woodLight;
            ctx.fillRect(p.x, p.y, p.w, 2);
        }
    },

    renderWall(ctx, w) {
        const area = this.getArea(w.x);
        let col1 = COL.stone, col2 = COL.stoneDark;
        if (area === 'corrupted' || area === 'proverbGate') {
            col1 = '#2A1545'; col2 = '#150A25';
        }

        const grad = ctx.createLinearGradient(w.x, w.y, w.x + w.w, w.y);
        grad.addColorStop(0, col2);
        grad.addColorStop(0.5, col1);
        grad.addColorStop(1, col2);
        ctx.fillStyle = grad;
        ctx.fillRect(w.x, w.y, w.w, w.h);

        // Brick pattern
        ctx.strokeStyle = 'rgba(0,0,0,0.12)';
        ctx.lineWidth = 1;
        let row = 0;
        for (let y = w.y; y < w.y + w.h; y += 20) {
            ctx.beginPath();
            ctx.moveTo(w.x, y);
            ctx.lineTo(w.x + w.w, y);
            ctx.stroke();
            const off = (row % 2) * 20;
            for (let x = w.x + off; x < w.x + w.w; x += 40) {
                ctx.beginPath();
                ctx.moveTo(x, y);
                ctx.lineTo(x, Math.min(y + 20, w.y + w.h));
                ctx.stroke();
            }
            row++;
        }

        // Corruption veins
        if ((area === 'corrupted' || area === 'proverbGate') && this.spiritBreached) {
            ctx.strokeStyle = 'rgba(160,80,255,0.12)';
            ctx.lineWidth = 1;
            for (let i = 0; i < 3; i++) {
                const sy = w.y + (w.h / 4) * (i + 1);
                ctx.beginPath();
                ctx.moveTo(w.x, sy);
                ctx.bezierCurveTo(
                    w.x + w.w * 0.3, sy - 15 + Math.sin(Game.time + i) * 5,
                    w.x + w.w * 0.7, sy + 10 + Math.cos(Game.time + i) * 5,
                    w.x + w.w, sy
                );
                ctx.stroke();
            }
        }
    },

    renderDecoration(ctx, d) {
        switch (d.type) {
            case 'banner': {
                const wave = Math.sin(Game.time * 2 + d.x * 0.01) * 5;
                ctx.fillStyle = d.color;
                ctx.beginPath();
                ctx.moveTo(d.x, d.y);
                ctx.lineTo(d.x + 25 + wave, d.y);
                ctx.lineTo(d.x + 20 + wave, d.y + 60);
                ctx.lineTo(d.x + 12, d.y + 50);
                ctx.lineTo(d.x + 5, d.y + 60);
                ctx.closePath();
                ctx.fill();
                ctx.fillStyle = COL.woodDark;
                ctx.fillRect(d.x - 2, d.y - 10, 4, 80);
                break;
            }
            case 'lantern': {
                const glow = Math.sin(Game.time * 3 + d.x) * 0.2 + 0.8;
                ctx.strokeStyle = 'rgba(100,80,60,0.4)';
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(d.x, d.y - 20);
                ctx.lineTo(d.x, d.y);
                ctx.stroke();
                ctx.fillStyle = `rgba(255,180,50,${glow * 0.7})`;
                ctx.fillRect(d.x - 6, d.y, 12, 16);
                ctx.fillStyle = `rgba(255,220,100,${glow})`;
                ctx.fillRect(d.x - 4, d.y + 2, 8, 12);
                // Glow radius
                const gg = ctx.createRadialGradient(d.x, d.y + 8, 2, d.x, d.y + 8, 45);
                gg.addColorStop(0, `rgba(255,180,50,${glow * 0.12})`);
                gg.addColorStop(1, 'rgba(255,180,50,0)');
                ctx.fillStyle = gg;
                ctx.fillRect(d.x - 45, d.y - 37, 90, 90);
                break;
            }
            case 'torch': {
                const fl = Math.sin(Game.time * 8 + d.x) * 0.2 + 0.8;
                ctx.fillStyle = '#4A3A28';
                ctx.fillRect(d.x - 3, d.y, 6, 30);
                // Flame
                ctx.fillStyle = `rgba(255,150,30,${fl})`;
                ctx.beginPath();
                ctx.moveTo(d.x, d.y - 14);
                ctx.quadraticCurveTo(d.x + 8, d.y - 4, d.x, d.y + 5);
                ctx.quadraticCurveTo(d.x - 8, d.y - 4, d.x, d.y - 14);
                ctx.fill();
                ctx.fillStyle = `rgba(255,220,80,${fl * 0.7})`;
                ctx.beginPath();
                ctx.moveTo(d.x, d.y - 9);
                ctx.quadraticCurveTo(d.x + 4, d.y - 2, d.x, d.y + 2);
                ctx.quadraticCurveTo(d.x - 4, d.y - 2, d.x, d.y - 9);
                ctx.fill();
                const tg = ctx.createRadialGradient(d.x, d.y - 5, 2, d.x, d.y - 5, 55);
                tg.addColorStop(0, `rgba(255,150,30,${fl * 0.1})`);
                tg.addColorStop(1, 'rgba(255,150,30,0)');
                ctx.fillStyle = tg;
                ctx.fillRect(d.x - 55, d.y - 60, 110, 110);
                break;
            }
            case 'drum': {
                ctx.fillStyle = COL.wood;
                ctx.fillRect(d.x - 12, d.y - 25, 24, 25);
                ctx.fillStyle = '#C0A060';
                ctx.fillRect(d.x - 14, d.y - 27, 28, 4);
                ctx.fillRect(d.x - 14, d.y - 2, 28, 4);
                ctx.strokeStyle = '#A08040';
                ctx.lineWidth = 1;
                for (let i = 0; i < 4; i++) {
                    ctx.beginPath();
                    ctx.moveTo(d.x - 10 + i * 7, d.y - 25);
                    ctx.lineTo(d.x - 7 + i * 7, d.y);
                    ctx.stroke();
                }
                break;
            }
            case 'stall': {
                const sw = d.w || 120;
                ctx.fillStyle = COL.wood;
                ctx.fillRect(d.x, d.y, 6, 100);
                ctx.fillRect(d.x + sw - 6, d.y, 6, 100);
                ctx.fillStyle = COL.festivalOrange;
                ctx.fillRect(d.x - 5, d.y - 10, sw + 10, 12);
                ctx.fillStyle = COL.woodLight;
                ctx.fillRect(d.x, d.y + 50, sw, 8);
                for (let i = 0; i < 4; i++) {
                    ctx.fillStyle = ['#E74C3C', '#F4D03F', '#27AE60', '#8E44AD'][i];
                    ctx.fillRect(d.x + 10 + i * 25, d.y + 35, 15, 12);
                }
                break;
            }
            case 'adinkra_wall': {
                this.drawAdinkra(ctx, d.x, d.y, d.symbol, 30);
                break;
            }
            case 'shrine_altar': {
                ctx.fillStyle = COL.stone;
                ctx.fillRect(d.x - 30, d.y, 60, 40);
                ctx.fillStyle = COL.stoneLight;
                ctx.fillRect(d.x - 35, d.y - 5, 70, 8);
                const ag = ctx.createRadialGradient(d.x, d.y, 5, d.x, d.y, 60);
                ag.addColorStop(0, 'rgba(212,175,55,0.15)');
                ag.addColorStop(1, 'rgba(212,175,55,0)');
                ctx.fillStyle = ag;
                ctx.fillRect(d.x - 60, d.y - 60, 120, 120);
                break;
            }
            case 'web_glow': {
                const wg = Math.sin(Game.time * 2) * 0.2 + 0.5;
                ctx.strokeStyle = `rgba(212,175,55,${wg * 0.25})`;
                ctx.lineWidth = 1;
                for (let i = 0; i < 8; i++) {
                    const angle = (i / 8) * Math.PI * 2;
                    ctx.beginPath();
                    ctx.moveTo(d.x, d.y);
                    ctx.lineTo(d.x + Math.cos(angle) * 100, d.y + Math.sin(angle) * 100);
                    ctx.stroke();
                }
                for (let r = 20; r <= 80; r += 20) {
                    ctx.beginPath();
                    ctx.arc(d.x, d.y, r, 0, Math.PI * 2);
                    ctx.strokeStyle = `rgba(212,175,55,${wg * 0.12})`;
                    ctx.stroke();
                }
                break;
            }
            case 'spirit_thread': {
                const st = Math.sin(Game.time * 1.5 + d.x * 0.01) * 10;
                ctx.strokeStyle = 'rgba(160,80,255,0.18)';
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(d.x, d.y + st);
                ctx.bezierCurveTo(d.x + d.w * 0.3, d.y - 20 + st, d.x + d.w * 0.7, d.y + 20 + st, d.x + d.w, d.y + st);
                ctx.stroke();
                break;
            }
            case 'floating_cloth': {
                const fy = Math.sin(Game.time + d.x * 0.02) * 15;
                const fx = Math.sin(Game.time * 0.7 + d.x * 0.03) * 5;
                ctx.fillStyle = 'rgba(160,80,255,0.12)';
                ctx.beginPath();
                ctx.moveTo(d.x + fx, d.y + fy);
                ctx.quadraticCurveTo(d.x + 15 + fx, d.y - 10 + fy, d.x + 30 + fx, d.y + 5 + fy);
                ctx.quadraticCurveTo(d.x + 15 + fx, d.y + 15 + fy, d.x + fx, d.y + fy);
                ctx.fill();
                break;
            }
            case 'cracked_adinkra': {
                this.drawAdinkra(ctx, d.x, d.y, 'cracked', 25);
                break;
            }
            case 'ceremonial_fire': {
                const ff = Math.sin(Game.time * 6 + d.x) * 0.3 + 0.7;
                ctx.fillStyle = COL.stone;
                ctx.fillRect(d.x - 15, d.y + 10, 30, 20);
                for (let i = 0; i < 3; i++) {
                    const fx = d.x + (i - 1) * 6;
                    const fh = 20 + Math.sin(Game.time * 10 + i + d.x) * 8;
                    ctx.fillStyle = `rgba(255,${120 + i * 40},30,${ff * 0.8})`;
                    ctx.beginPath();
                    ctx.moveTo(fx - 5, d.y + 10);
                    ctx.quadraticCurveTo(fx, d.y + 10 - fh, fx + 5, d.y + 10);
                    ctx.fill();
                }
                const fg = ctx.createRadialGradient(d.x, d.y, 5, d.x, d.y, 70);
                fg.addColorStop(0, `rgba(255,150,30,${ff * 0.08})`);
                fg.addColorStop(1, 'rgba(255,150,30,0)');
                ctx.fillStyle = fg;
                ctx.fillRect(d.x - 70, d.y - 60, 140, 140);
                break;
            }
            case 'building_face': {
                const isCorr = this.getArea(d.x) === 'corrupted';
                ctx.fillStyle = isCorr ? 'rgba(80,30,150,0.25)' : 'rgba(255,200,80,0.12)';
                ctx.fillRect(d.x + d.w * 0.2, d.y + 40, d.w * 0.25, d.h * 0.08);
                ctx.fillRect(d.x + d.w * 0.55, d.y + 40, d.w * 0.25, d.h * 0.08);
                ctx.fillStyle = COL.woodDark;
                ctx.fillRect(d.x + d.w * 0.3, d.y + d.h - 50, d.w * 0.4, 50);
                break;
            }
            case 'proverb_gate': {
                ctx.fillStyle = COL.stone;
                ctx.fillRect(d.x, d.y, d.w, d.h);
                // Gold trim
                ctx.fillStyle = COL.gold;
                ctx.fillRect(d.x + 3, d.y + 3, d.w - 6, 3);
                ctx.fillRect(d.x + 3, d.y + d.h - 6, d.w - 6, 3);
                ctx.fillRect(d.x + 3, d.y, 3, d.h);
                ctx.fillRect(d.x + d.w - 6, d.y, 3, d.h);
                if (!Puzzle.solved) {
                    ctx.fillStyle = '#2A1A0A';
                    ctx.fillRect(d.x + 15, d.y + 60, d.w - 30, d.h - 60);
                    // Symbols on gate
                    this.drawAdinkra(ctx, d.x + 50, d.y + 140, 'gye_nyame', 18);
                    this.drawAdinkra(ctx, d.x + 50, d.y + 210, 'sankofa', 18);
                    this.drawAdinkra(ctx, d.x + 50, d.y + 280, 'dwennimmen', 18);
                } else {
                    // Gate open — just the frame
                }
                break;
            }
        }
    },

    drawAdinkra(ctx, x, y, symbol, size) {
        const s = size || 20;
        const isCorrupted = symbol === 'cracked';
        const alpha = Player.spiderSenseActive ? 0.9 : 0.4;
        ctx.strokeStyle = isCorrupted ? `rgba(160,80,255,${alpha})` : `rgba(212,175,55,${alpha})`;
        ctx.lineWidth = 2;

        if (symbol === 'gye_nyame' || symbol === 'gate1') {
            ctx.beginPath();
            ctx.arc(x, y, s * 0.6, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(x - s * 0.4, y);
            ctx.lineTo(x + s * 0.4, y);
            ctx.moveTo(x, y - s * 0.4);
            ctx.lineTo(x, y + s * 0.4);
            ctx.stroke();
        } else if (symbol === 'sankofa' || symbol === 'gate2') {
            ctx.beginPath();
            ctx.moveTo(x - s * 0.4, y + s * 0.3);
            ctx.quadraticCurveTo(x - s * 0.5, y - s * 0.4, x, y - s * 0.3);
            ctx.quadraticCurveTo(x + s * 0.5, y - s * 0.4, x + s * 0.4, y + s * 0.3);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(x, y + s * 0.1, s * 0.15, 0, Math.PI * 2);
            ctx.stroke();
        } else if (symbol === 'dwennimmen' || symbol === 'gate3') {
            ctx.beginPath();
            ctx.moveTo(x, y - s * 0.4);
            ctx.lineTo(x - s * 0.4, y);
            ctx.lineTo(x, y + s * 0.4);
            ctx.lineTo(x + s * 0.4, y);
            ctx.closePath();
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(x - s * 0.2, y - s * 0.2);
            ctx.lineTo(x + s * 0.2, y + s * 0.2);
            ctx.moveTo(x + s * 0.2, y - s * 0.2);
            ctx.lineTo(x - s * 0.2, y + s * 0.2);
            ctx.stroke();
        } else if (isCorrupted) {
            ctx.beginPath();
            ctx.arc(x, y, s * 0.5, 0, Math.PI * 2);
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(x - s * 0.2, y - s * 0.3);
            ctx.lineTo(x + s * 0.1, y);
            ctx.lineTo(x - s * 0.1, y + s * 0.3);
            ctx.stroke();
        }

        if (Player.spiderSenseActive) {
            const glow = ctx.createRadialGradient(x, y, 0, x, y, s);
            glow.addColorStop(0, 'rgba(212,175,55,0.15)');
            glow.addColorStop(1, 'rgba(212,175,55,0)');
            ctx.fillStyle = glow;
            ctx.fillRect(x - s, y - s, s * 2, s * 2);
        }
    }
};

// ============ PLAYER ============
const Player = {
    x: 100, y: 600,
    vx: 0, vy: 0,
    w: 24, h: 48,
    facing: 1,
    grounded: false,
    health: 100, maxHealth: 100,
    energy: 100, maxEnergy: 100,
    speed: 220,
    jumpForce: -12.5,

    // States
    state: 'idle',
    stateTimer: 0,
    invulnerable: false, invulnTimer: 0,

    // Combat
    attackCombo: 0,
    attackTimer: 0,
    comboCount: 0,
    comboTimer: 0,
    maxCombo: 0,
    parryWindow: 0,
    parryActive: false,

    // Stealth
    isCrouching: false,
    isHidden: false,
    noiseLevel: 0,

    // Climbing
    climbing: false,
    climbSurface: null,

    // Abilities
    abilities: { silkenPath: false, spiderSense: false, trickMirror: false, borrowedVoice: false },
    cooldowns: { e: 0, r: 0, f: 0, v: 0 },
    spiderSenseActive: false,
    spiderSenseTimer: 0,
    decoys: [],

    // Latch
    latching: false,
    latchTarget: null,
    latchProgress: 0,
    webLine: null,

    // Dodge
    dodgeTimer: 0,
    dodgeDir: 0,
    dodgeCooldown: 0,

    // Anim
    animFrame: 0,
    animTimer: 0,
    spiderMark: false,
    _lastArea: '',

    init() {
        this.x = 150;
        this.y = 600;
        this.health = 100;
        this.energy = 100;
    },

    update(dt) {
        if (Game.state === 'cutscene' && Cinematic.lockPlayer) {
            this.vy += GRAVITY;
            this.applyPhysics(dt);
            this.animate(dt);
            return;
        }

        this.animTimer += dt;

        // Cooldowns
        for (const k of Object.keys(this.cooldowns)) {
            if (this.cooldowns[k] > 0) this.cooldowns[k] -= dt;
        }

        if (this.invulnerable) {
            this.invulnTimer -= dt;
            if (this.invulnTimer <= 0) this.invulnerable = false;
        }

        // Always tick down attack timer so it doesn't get stuck after being interrupted by hurt
        if (this.attackTimer > 0) this.attackTimer -= dt;

        if (this.comboTimer > 0) {
            this.comboTimer -= dt;
            if (this.comboTimer <= 0) this.comboCount = 0;
        }

        if (this.spiderSenseActive) {
            this.spiderSenseTimer -= dt;
            if (this.spiderSenseTimer <= 0) this.spiderSenseActive = false;
        }

        // Decoys
        for (let i = this.decoys.length - 1; i >= 0; i--) {
            this.decoys[i].life -= dt;
            if (this.decoys[i].life <= 0) this.decoys.splice(i, 1);
        }

        // Dodge
        if (this.state === 'dodge') {
            this.dodgeTimer -= dt;
            this.vx = this.dodgeDir * 420;
            this.invulnerable = true;
            this.invulnTimer = 0.05;
            if (this.dodgeTimer <= 0) {
                this.state = 'idle';
                this.vx = 0;
            }
            this.vy += GRAVITY;
            this.applyPhysics(dt);
            this.animate(dt);
            return;
        }

        // Attack
        if (this.state === 'attack') {
            this.attackTimer -= dt;
            if (this.attackTimer <= 0) this.state = 'idle';
            this.vy += GRAVITY;
            this.applyPhysics(dt);
            this.animate(dt);
            return;
        }

        // Hurt
        if (this.state === 'hurt') {
            this.stateTimer -= dt;
            if (this.stateTimer <= 0) this.state = 'idle';
            this.vy += GRAVITY;
            this.applyPhysics(dt);
            this.animate(dt);
            return;
        }

        // Latch
        if (this.latching && this.latchTarget) {
            this.latchProgress += dt * 5;
            const t = Math.min(this.latchProgress, 1);
            this.x = lerp(this._latchStartX, this.latchTarget.x, t);
            this.y = lerp(this._latchStartY, this.latchTarget.y, t);
            this.vy = 0;
            this.vx = 0;
            this.webLine = { x1: this.x, y1: this.y + this.h / 2, x2: this.latchTarget.x, y2: this.latchTarget.y };
            if (t >= 1) {
                this.latching = false;
                this.latchTarget = null;
                this.webLine = null;
                this.vy = -10;
            }
            this.animate(dt);
            return;
        }
        this.webLine = null;

        // Climbing
        if (this.climbing && this.climbSurface) {
            this.vy = 0;
            this.vx = 0;
            if (Input.up) this.y -= 140 * dt;
            if (Input.down) this.y += 100 * dt;
            // Jump off wall
            if (Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp')) {
                // Only wall-jump if also pressing away
                if (Input.left || Input.right) {
                    this.climbing = false;
                    this.climbSurface = null;
                    this.vy = this.jumpForce * 0.8;
                    this.vx = (Input.left ? -1 : 1) * 200;
                    this.facing = Input.left ? -1 : 1;
                    Audio.playJump();
                }
            }
            // Reach top
            if (this.y <= this.climbSurface.y - 10) {
                this.climbing = false;
                this.climbSurface = null;
                this.y -= 10;
                this.vy = -4;
            }
            this.animate(dt);
            return;
        }

        // --- Input ---
        let moveX = 0;
        if (Input.left) moveX -= 1;
        if (Input.right) moveX += 1;

        this.isCrouching = Input.crouch && this.grounded;
        const moveSpeed = this.isCrouching ? this.speed * 0.4 : this.speed;

        if (moveX !== 0) {
            this.facing = moveX;
            this.vx = moveX * moveSpeed;
            this.state = this.isCrouching ? 'crouch' : (this.grounded ? 'run' : this.state);
            this.noiseLevel = this.isCrouching ? 0.2 : 0.6;
        } else {
            this.vx *= 0.75;
            if (Math.abs(this.vx) < 5) this.vx = 0;
            this.state = this.isCrouching ? 'crouch' : (this.grounded ? 'idle' : this.state);
            this.noiseLevel = 0;
        }

        this.isHidden = this.isCrouching && !!World.getHideSpot(this.x, this.y + this.h / 2);

        // NPC interact (checked BEFORE jump so W talks instead of jumping near NPCs)
        if ((Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp')) && this.grounded && Game.state === 'playing') {
            const npc = NPCManager.getNearby(this.x, this.y + this.h / 2, 60);
            if (npc && npc.dialogue) {
                Dialogue.start(npc.dialogue, npc.name, npc.color);
                return;
            }

            // Puzzle symbol interact (also before jump)
            if (Puzzle.started && !Puzzle.solved) {
                for (let i = 0; i < Puzzle.symbolPositions.length; i++) {
                    const sp = Puzzle.symbolPositions[i];
                    if (!sp.activated && Math.abs(this.x - sp.x) < 45 && Math.abs(this.y + this.h - sp.y) < 60) {
                        Puzzle.activateSymbol(i);
                        return;
                    }
                }
            }
        }

        // Jump
        if ((Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp')) && this.grounded && !this.isCrouching) {
            this.vy = this.jumpForce;
            this.grounded = false;
            this.state = 'jump';
            Audio.playJump();
        }

        // Wall climb
        if (!this.grounded && moveX !== 0 && !this.isCrouching) {
            const cs = World.getClimbSurface(this.x + this.facing * (this.w / 2 + 5), this.y + this.h / 2);
            if (cs) {
                this.climbing = true;
                this.climbSurface = cs;
                this.vy = 0;
                this.vx = 0;
                return;
            }
        }

        // Dodge
        if (Input.dodge && this.grounded && this.dodgeCooldown <= 0) {
            this.state = 'dodge';
            this.dodgeTimer = 0.25;
            this.dodgeDir = this.facing;
            this.dodgeCooldown = 0.4;
            Audio.playDodge();
            ParticleSystem.emit(this.x, this.y + this.h, 5, {
                color: 'rgba(200,180,140,0.5)', life: 0.3, size: 4, speed: 50, gravity: 0.1
            });
            return;
        }
        if (this.dodgeCooldown > 0) this.dodgeCooldown -= dt;

        // Attack
        if (Input.attack) {
            // Check stealth kill first
            if (this.isCrouching) {
                const enemy = EnemyManager.getNearby(this.x, this.y + this.h / 2, 55);
                if (enemy && !enemy.alert) {
                    this.performStealthKill(enemy);
                    return;
                }
            }
            this.performAttack();
        }

        // Parry
        if (Input.parry) {
            this.parryActive = true;
            this.parryWindow = 0.2;
            // Parry visual
            ParticleSystem.emit(this.x + this.facing * 15, this.y + 20, 3, {
                color: COL.goldBright, life: 0.15, size: 6, speed: 20
            });
        }
        if (this.parryActive) {
            this.parryWindow -= dt;
            if (this.parryWindow <= 0) this.parryActive = false;
        }

        // Abilities
        if (Input.silkenPath && this.abilities.silkenPath && this.cooldowns.e <= 0 && this.energy >= 15) {
            this.useSilkenPath();
        }
        if (Input.spiderSense && this.abilities.spiderSense && this.cooldowns.r <= 0 && this.energy >= 20) {
            this.useSpiderSense();
        }
        if (Input.trickMirror && this.abilities.trickMirror && this.cooldowns.f <= 0 && this.energy >= 25) {
            this.useTrickMirror();
        }
        if (Input.borrowedVoice && this.abilities.borrowedVoice && this.cooldowns.v <= 0 && this.energy >= 10) {
            this.useBorrowedVoice();
        }

        // Energy regen
        this.energy = Math.min(this.maxEnergy, this.energy + 6 * dt);

        // Gravity
        if (!this.grounded) {
            this.vy += GRAVITY;
            this.state = this.vy > 0 ? 'fall' : 'jump';
        }

        this.applyPhysics(dt);
        this.animate(dt);
        this.checkAreaTransitions();
    },

    applyPhysics(dt) {
        this.x += this.vx * dt;

        // Wall collisions
        for (const wall of World.walls) {
            if (this.x + this.w / 2 > wall.x && this.x - this.w / 2 < wall.x + wall.w &&
                this.y + this.h > wall.y && this.y < wall.y + wall.h) {
                if (this.vx > 0) this.x = wall.x - this.w / 2;
                else if (this.vx < 0) this.x = wall.x + wall.w + this.w / 2;
                this.vx = 0;
            }
        }

        this.x = Math.max(this.w / 2, Math.min(WORLD_W - this.w / 2, this.x));

        this.y += this.vy;
        this.grounded = false;

        for (const p of World.platforms) {
            if (this.x + this.w / 2 > p.x && this.x - this.w / 2 < p.x + p.w) {
                if (this.y + this.h >= p.y && this.y + this.h <= p.y + p.h + 12 && this.vy >= 0) {
                    this.y = p.y - this.h;
                    this.vy = 0;
                    this.grounded = true;
                }
            }
        }

        if (this.y + this.h > WORLD_H + 50) {
            this.y = WORLD_H + 50 - this.h;
            this.vy = 0;
            this.grounded = true;
        }
    },

    performAttack() {
        if (this.attackTimer > 0) return;

        this.state = 'attack';
        this.attackCombo = (this.attackCombo + 1) % 3;
        this.attackTimer = 0.22;

        const range = 55;
        const attackX = this.x + this.facing * (range * 0.6);
        const attackY = this.y + this.h * 0.4;

        let hitSomething = false;

        // Hit enemies
        for (const e of EnemyManager.enemies) {
            if (e.dead) continue;
            const dx = Math.abs(e.x - attackX);
            const dy = Math.abs((e.y + e.h / 2) - attackY);
            if (dx < range && dy < 60) {
                const dmg = 15 + this.attackCombo * 5;
                e.takeDamage(dmg, this.facing);
                hitSomething = true;
                Game.triggerSlowMo(0.1, 0.04);
                Game.shake(4, 0.25);
                this.comboCount++;
                this.comboTimer = 2;
                if (this.comboCount > this.maxCombo) {
                    this.maxCombo = this.comboCount;
                    Game.stats.maxCombo = this.maxCombo;
                }
                ParticleSystem.emit(e.x, e.y + e.h * 0.4, 6, {
                    color: COL.gold, life: 0.3, size: 4, speed: 100, gravity: 0.3
                });
            }
        }

        // Boss hit
        if (BossManager.active && BossManager.boss && !BossManager.boss.dead) {
            const b = BossManager.boss;
            const dx = Math.abs(b.x - attackX);
            const dy = Math.abs((b.y + b.h / 2) - attackY);
            if (dx < range + 20 && dy < 80) {
                BossManager.takeDamage(10 + this.attackCombo * 4);
                hitSomething = true;
                Game.triggerSlowMo(0.15, 0.05);
                Game.shake(5, 0.3);
                this.comboCount++;
                this.comboTimer = 2;
                if (this.comboCount > this.maxCombo) {
                    this.maxCombo = this.comboCount;
                    Game.stats.maxCombo = this.maxCombo;
                }
            }
        }

        // Boss illusions
        for (let i = BossManager.illusions.length - 1; i >= 0; i--) {
            const ill = BossManager.illusions[i];
            const dx = Math.abs(ill.x - attackX);
            const dy = Math.abs((ill.y + ill.h / 2) - attackY);
            if (dx < range + 10 && dy < 70) {
                ill.health -= 15;
                ill._hitFlash = 1;
                hitSomething = true;
                if (ill.health <= 0) {
                    ParticleSystem.emit(ill.x, ill.y + 30, 15, {
                        color: COL.corruption, life: 0.5, size: 3, speed: 80
                    });
                    BossManager.illusions.splice(i, 1);
                }
            }
        }

        Audio[hitSomething ? 'playHit' : 'playSwing']();

        // Attack swoosh
        ParticleSystem.emit(this.x + this.facing * 30, this.y + 20, 3, {
            color: 'rgba(200,180,140,0.4)', life: 0.15, size: 3, speed: 60
        });
    },

    performStealthKill(enemy) {
        this.state = 'attack';
        this.attackTimer = 0.4;
        enemy.die();
        Game.stats.stealth++;
        Game.triggerSlowMo(0.25, 0.4);
        Game.shake(3, 0.2);
        Audio.playStealthKill();
        ParticleSystem.emit(enemy.x, enemy.y + enemy.h / 2, 12, {
            color: COL.gold, life: 0.6, size: 3, speed: 90, gravity: 0.2
        });
    },

    useSilkenPath() {
        this.energy -= 15;
        this.cooldowns.e = 2;
        Audio.playWeb();

        if (!this.grounded) {
            const lp = World.getLatchPoint(this.x, this.y, 280);
            if (lp) {
                this.latching = true;
                this.latchTarget = lp;
                this.latchProgress = 0;
                this._latchStartX = this.x;
                this._latchStartY = this.y;
                return;
            }
        }

        // Ground: snare
        const enemy = EnemyManager.getNearby(this.x + this.facing * 80, this.y + this.h / 2, 120);
        if (enemy && !enemy.dead) {
            enemy.snared = true;
            enemy.snareTimer = 2.5;
            enemy.vx = 0;
            // Pull slightly
            enemy.x += Math.sign(this.x - enemy.x) * 20;
            ParticleSystem.emit(enemy.x, enemy.y + enemy.h / 2, 8, {
                color: 'rgba(212,175,55,0.5)', life: 0.5, size: 2, speed: 40
            });
        } else {
            ParticleSystem.emit(this.x + this.facing * 60, this.y + this.h / 2, 6, {
                color: COL.gold, life: 0.6, size: 2, speed: 40
            });
        }
    },

    useSpiderSense() {
        this.energy -= 20;
        this.cooldowns.r = 5;
        this.spiderSenseActive = true;
        this.spiderSenseTimer = 5;
        Game.triggerSlowMo(0.4, 0.6);
        Audio.playSpiderSense();
        ParticleSystem.emit(this.x, this.y + this.h / 2, 24, {
            color: COL.gold, life: 1.2, size: 2, speed: 160, gravity: 0, fadeOut: true
        });
    },

    useTrickMirror() {
        this.energy -= 25;
        this.cooldowns.f = 8;
        this.decoys.push({ x: this.x, y: this.y, facing: this.facing, life: 5 });
        Audio.playDecoy();
        ParticleSystem.emit(this.x, this.y + this.h / 2, 8, {
            color: 'rgba(160,120,255,0.4)', life: 0.5, size: 3, speed: 60
        });
    },

    useBorrowedVoice() {
        this.energy -= 10;
        this.cooldowns.v = 4;
        const enemy = EnemyManager.getNearby(this.x + this.facing * 150, this.y + this.h / 2, 220);
        if (enemy && enemy.type === 'guard') {
            enemy.lured = true;
            enemy.lureX = this.x + this.facing * 200;
            enemy.lureTimer = 3;
            Audio.playVoiceLure();
            ParticleSystem.emit(enemy.lureX, enemy.y - 20, 5, {
                color: 'rgba(100,200,255,0.5)', life: 0.8, size: 3, speed: 25
            });
        }
    },

    takeDamage(amount) {
        if (this.invulnerable || this.state === 'dodge') return;

        if (this.parryActive) {
            Audio.playParry();
            Game.triggerSlowMo(0.1, 0.35);
            Game.shake(5, 0.35);
            ParticleSystem.emit(this.x + this.facing * 15, this.y + 20, 12, {
                color: COL.goldBright, life: 0.4, size: 5, speed: 130
            });
            this.parryActive = false;
            // Parry stuns nearby enemies briefly
            for (const e of EnemyManager.enemies) {
                if (!e.dead && Math.hypot(e.x - this.x, e.y - this.y) < 80) {
                    e.snared = true;
                    e.snareTimer = 1;
                }
            }
            return;
        }

        this.health -= amount;
        this.state = 'hurt';
        this.stateTimer = 0.3;
        this.invulnerable = true;
        this.invulnTimer = 0.6;
        this.comboCount = 0;
        Game.shake(6, 0.4);
        Audio.playHurt();

        ParticleSystem.emit(this.x, this.y + 20, 8, {
            color: COL.red, life: 0.4, size: 3, speed: 80
        });

        if (this.health <= 0) {
            this.health = 0;
            this.respawn();
        }
    },

    respawn() {
        this.health = this.maxHealth;
        this.energy = this.maxEnergy;
        const area = World.getArea(this.x);
        const areaData = AREAS[area];
        if (areaData) {
            this.x = areaData.x + 80;
            this.y = 600;
        }
        this.invulnerable = true;
        this.invulnTimer = 2;
        this.state = 'idle';
    },

    checkAreaTransitions() {
        const area = World.getArea(this.x);
        if (area !== this._lastArea) {
            this._lastArea = area;
            const areaData = AREAS[area];
            if (areaData && areaData.name) {
                HUD.showAreaName(areaData.name, areaData.sub);
            }

            if (area === 'shrine' && !this._shrineVisited) {
                this._shrineVisited = true;
                Cinematic.startShrine();
            }
            if (area === 'proverbGate' && !Puzzle.started && World.spiritBreached) {
                Puzzle.start();
            }
            if (area === 'bossArena' && !BossManager.active && !BossManager.defeated && Puzzle.solved) {
                BossManager.start();
            }
            if (area === 'ending' && BossManager.defeated && !Cinematic._endingStarted) {
                Cinematic._endingStarted = true;
                Cinematic.startEnding();
            }
        }
    },

    animate(dt) {
        if (this.animTimer > 0.08) {
            this.animTimer = 0;
            this.animFrame = (this.animFrame + 1) % 8;
        }
    },

    render(ctx) {
        if (this.invulnerable && Math.floor(Game.time * 20) % 2 === 0 && this.state !== 'dodge') return;

        ctx.save();
        ctx.translate(this.x, this.y);
        if (this.facing < 0) ctx.scale(-1, 1);

        const cr = this.isCrouching;
        const bH = cr ? 30 : 48;
        const bY = cr ? 18 : 0;

        // Shadow
        ctx.fillStyle = 'rgba(0,0,0,0.3)';
        ctx.beginPath();
        ctx.ellipse(0, this.h, 14, 4, 0, 0, Math.PI * 2);
        ctx.fill();

        // Dodge after-image
        if (this.state === 'dodge') {
            ctx.globalAlpha = 0.25;
            ctx.fillStyle = COL.gold;
            ctx.fillRect(-this.w / 2 - this.facing * 20, bY + 5, this.w, bH - 10);
            ctx.globalAlpha = 1;
        }

        // Legs
        const legAnim = (this.state === 'run') ? Math.sin(Game.time * 14) * 6 : 0;
        ctx.fillStyle = '#5A3E1A';
        ctx.fillRect(-5, bH + bY - 4, 5, cr ? 5 : 10);
        ctx.fillRect(2, bH + bY - 4 + legAnim * 0.5, 5, cr ? 5 : 10);

        // Feet
        ctx.fillStyle = '#3E2A10';
        ctx.fillRect(-6, this.h - 3, 6, 3);
        ctx.fillRect(1, this.h - 3 + legAnim * 0.3, 6, 3);

        // Body
        ctx.fillStyle = COL.cloth1;
        ctx.fillRect(-this.w / 2 + 2, bY + 8, this.w - 4, bH - 18);

        // Cloth wrap detail
        ctx.strokeStyle = '#C05A20';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(-this.w / 2 + 3, bY + 14);
        ctx.lineTo(this.w / 2 - 3, bY + 20);
        ctx.stroke();
        ctx.strokeStyle = COL.cloth2;
        ctx.beginPath();
        ctx.moveTo(-this.w / 2 + 3, bY + 22);
        ctx.lineTo(this.w / 2 - 3, bY + 28);
        ctx.stroke();

        // Left arm (back)
        ctx.fillStyle = COL.skinWarm;
        ctx.fillRect(-10, bY + 10, 5, 14);

        // Right arm + weapon
        const armAngle = this.state === 'attack' ?
            [0.9, -0.6, 1.3][this.attackCombo] : (this.state === 'run' ? Math.sin(Game.time * 14) * 0.3 : 0);
        ctx.save();
        ctx.translate(8, bY + 12);
        ctx.rotate(armAngle);
        ctx.fillStyle = COL.skinWarm;
        ctx.fillRect(0, -2, 14, 5);
        // Weapon
        if (this.state === 'attack') {
            ctx.fillStyle = COL.woodDark;
            ctx.fillRect(12, -1, 18, 3);
            ctx.fillStyle = '#B8B8B8';
            ctx.fillRect(28, -3, 10, 7);
            // Edge gleam
            ctx.fillStyle = '#E0E0E0';
            ctx.fillRect(36, -2, 2, 5);
        } else {
            ctx.fillStyle = COL.woodDark;
            ctx.fillRect(0, 3, 3, 18);
        }
        ctx.restore();

        // Head
        ctx.fillStyle = COL.skinWarm;
        ctx.beginPath();
        ctx.arc(0, bY + 4, 9, 0, Math.PI * 2);
        ctx.fill();

        // Hair
        ctx.fillStyle = '#1A1A1A';
        ctx.beginPath();
        ctx.arc(0, bY + 1, 9, Math.PI + 0.3, Math.PI * 2 - 0.3);
        ctx.fill();

        // Eyes
        ctx.fillStyle = '#FFF';
        ctx.fillRect(2, bY + 2, 4, 3);
        ctx.fillStyle = '#111';
        ctx.fillRect(3, bY + 3, 2, 2);

        // Confident expression line
        ctx.strokeStyle = COL.skinDark;
        ctx.lineWidth = 0.8;
        ctx.beginPath();
        ctx.moveTo(2, bY + 8);
        ctx.lineTo(5, bY + 8);
        ctx.stroke();

        // Spider mark
        if (this.spiderMark) {
            const pulse = Math.sin(Game.time * 3) * 0.3 + 0.7;
            ctx.fillStyle = `rgba(212,175,55,${pulse * 0.8})`;
            ctx.beginPath();
            ctx.arc(14, bY + 14, 3, 0, Math.PI * 2);
            ctx.fill();
            // Tiny web lines from mark
            if (pulse > 0.7) {
                ctx.strokeStyle = `rgba(212,175,55,${(pulse - 0.7) * 2})`;
                ctx.lineWidth = 0.5;
                for (let i = 0; i < 4; i++) {
                    const a = (i / 4) * Math.PI * 2 + Game.time;
                    ctx.beginPath();
                    ctx.moveTo(14, bY + 14);
                    ctx.lineTo(14 + Math.cos(a) * 6, bY + 14 + Math.sin(a) * 6);
                    ctx.stroke();
                }
            }
        }

        // Hidden indicator
        if (this.isHidden) {
            ctx.strokeStyle = 'rgba(100,140,220,0.5)';
            ctx.lineWidth = 1.5;
            ctx.setLineDash([3, 3]);
            ctx.strokeRect(-this.w / 2 - 4, bY - 4, this.w + 8, bH + 8);
            ctx.setLineDash([]);
        }

        ctx.restore();

        // Web line
        if (this.webLine) {
            ctx.strokeStyle = COL.gold;
            ctx.lineWidth = 2;
            ctx.globalAlpha = 0.7;
            ctx.beginPath();
            ctx.moveTo(this.webLine.x1, this.webLine.y1);
            ctx.lineTo(this.webLine.x2, this.webLine.y2);
            ctx.stroke();
            ctx.globalAlpha = 1;
        }

        // Decoys
        for (const decoy of this.decoys) {
            const alpha = (decoy.life / 5) * 0.4;
            ctx.globalAlpha = alpha;
            ctx.save();
            ctx.translate(decoy.x, decoy.y);
            if (decoy.facing < 0) ctx.scale(-1, 1);
            ctx.fillStyle = 'rgba(160,120,255,0.3)';
            ctx.fillRect(-this.w / 2, 0, this.w, this.h);
            ctx.fillStyle = 'rgba(160,120,255,0.5)';
            ctx.beginPath();
            ctx.arc(0, 5, 9, 0, Math.PI * 2);
            ctx.fill();
            // Shimmer
            ctx.strokeStyle = 'rgba(212,175,55,0.3)';
            ctx.lineWidth = 1;
            ctx.strokeRect(-this.w / 2 - 2, -2, this.w + 4, this.h + 4);
            ctx.restore();
            ctx.globalAlpha = 1;
        }
    }
};

// ============ NPC MANAGER ============
const NPCManager = {
    npcs: [],

    init() {
        this.npcs = [];
        // Festival NPCs
        this.npcs.push({
            name: 'Market Vendor',
            x: AREAS.festival.x + 260, y: 655,
            w: 20, h: 40,
            color: COL.festivalGold,
            clothColor: '#E74C3C',
            dialogue: [
                { speaker: 'Market Vendor', text: "Welcome, friend! The Festival of the New Moon is upon us. Have you heard the elder's tale tonight? They say it speaks of Anansi himself." },
                { speaker: 'Kweku', text: "(The old spider stories... I've heard them a thousand times.)", choices: [
                    { text: "What's special about tonight's tale?", next: 2 },
                    { text: "I'm more interested in your wares.", next: 3 }
                ]},
                { speaker: 'Market Vendor', text: "Ah, tonight the storyteller weaves a tale never told before. They say he found a fragment of the First Story. The one Anansi himself hid from the world. Best hurry to the stage!" },
                { speaker: 'Market Vendor', text: "Ha! A trader's heart in one so young. But even I will close my stall soon. The tale begins any moment. Do not miss it." }
            ],
            active: true, facing: 1
        });

        this.npcs.push({
            name: 'Old Drummer',
            x: AREAS.festival.x + 1010, y: 655,
            w: 20, h: 40,
            color: '#A08050',
            clothColor: '#8E44AD',
            dialogue: [
                { speaker: 'Old Drummer', text: "My drums feel strange tonight. The rhythms want to speak a pattern I have never played. Something stirs in the old places." },
                { speaker: 'Kweku', text: "(He seems unsettled...)", choices: [
                    { text: "What kind of pattern?", next: 2 },
                    { text: "It's just the excitement of the festival.", next: 3 }
                ]},
                { speaker: 'Old Drummer', text: "A pattern of threes. Three symbols, three truths, three prices. Remember this: 'The return of the bird, the wisdom of the ram, and the eye of God... opens doors meant to stay closed.'" },
                { speaker: 'Old Drummer', text: "Perhaps, perhaps. But my hands have played forty festivals. They know when the air itself is listening. Go, watch the storyteller." }
            ],
            active: true, facing: -1
        });

        this.npcs.push({
            name: 'Dancing Woman',
            x: AREAS.festival.x + 700, y: 658,
            w: 18, h: 38,
            color: COL.skinWarm,
            clothColor: COL.festivalOrange,
            dialogue: [
                { speaker: 'Dancing Woman', text: "Dance with us! Tonight, the spirits of our ancestors watch over the festival. Can you feel their joy?" }
            ],
            active: true, facing: 1
        });

        // Storyteller
        this.npcs.push({
            name: 'Storyteller',
            x: AREAS.festival.x + 1175, y: 598,
            w: 22, h: 42,
            color: '#C0A060',
            clothColor: '#FFFFFF',
            dialogue: null,
            active: true, facing: -1,
            isStoryteller: true
        });
    },

    update(dt) {
        for (const npc of this.npcs) {
            if (!npc.active) continue;
            npc._sway = Math.sin(Game.time * 1.8 + npc.x * 0.1) * 2;
            const dist = Math.hypot(npc.x - Player.x, npc.y - (Player.y + Player.h / 2));
            npc._showBubble = dist < 80 && npc.dialogue && Game.state === 'playing';
        }
    },

    getNearby(x, y, range) {
        for (const npc of this.npcs) {
            if (!npc.active || !npc.dialogue) continue;
            if (Math.abs(npc.x - x) < range && Math.abs(npc.y - y) < range) return npc;
        }
        return null;
    },

    render(ctx) {
        for (const npc of this.npcs) {
            if (!npc.active) continue;
            if (npc.x < Camera.x - 100 || npc.x > Camera.x + Game.width + 100) continue;

            ctx.save();
            ctx.translate(npc.x, npc.y);
            if (npc.facing < 0) ctx.scale(-1, 1);

            const sway = npc._sway || 0;

            // Shadow
            ctx.fillStyle = 'rgba(0,0,0,0.2)';
            ctx.beginPath();
            ctx.ellipse(0, npc.h, 10, 3, 0, 0, Math.PI * 2);
            ctx.fill();

            // Body
            ctx.fillStyle = npc.clothColor;
            ctx.fillRect(-npc.w / 2 + sway * 0.5, 10, npc.w, npc.h - 18);

            // Legs
            ctx.fillStyle = '#4A3620';
            ctx.fillRect(-4, npc.h - 8, 4, 8);
            ctx.fillRect(2, npc.h - 8, 4, 8);

            // Head
            ctx.fillStyle = npc.color;
            ctx.beginPath();
            ctx.arc(sway * 0.3, 6, 8, 0, Math.PI * 2);
            ctx.fill();

            // Hair
            if (npc.isStoryteller) {
                ctx.fillStyle = '#E0E0E0';
                ctx.beginPath();
                ctx.arc(sway * 0.3, 3, 9, Math.PI, Math.PI * 2);
                ctx.fill();
            } else {
                ctx.fillStyle = '#1A1A1A';
                ctx.beginPath();
                ctx.arc(sway * 0.3, 3, 8, Math.PI, Math.PI * 2);
                ctx.fill();
            }

            // Eyes
            ctx.fillStyle = '#111';
            ctx.fillRect(2 + sway * 0.2, 5, 2, 2);

            ctx.restore();

            // Bubble
            if (npc._showBubble) {
                const bx = npc.x;
                const by = npc.y - 22;
                ctx.fillStyle = 'rgba(10,6,16,0.7)';
                ctx.strokeStyle = 'rgba(212,175,55,0.5)';
                ctx.lineWidth = 1;
                roundRect(ctx, bx - 14, by - 8, 28, 16, 4);
                ctx.fill();
                ctx.stroke();
                ctx.fillStyle = 'rgba(212,175,55,0.9)';
                ctx.font = '11px Rajdhani, sans-serif';
                ctx.textAlign = 'center';
                ctx.fillText('...', bx, by + 5);
            }
        }
    }
};

// ============ ENEMY MANAGER ============
const EnemyManager = {
    enemies: [],

    init() {
        this.enemies = [];
    },

    spawnGuard(x, y, patrolRange) {
        const e = {
            type: 'guard', x, y: y, w: 22, h: 46,
            vx: 0, vy: 0,
            health: 40, maxHealth: 40,
            damage: 12,
            speed: 65,
            facing: 1,
            state: 'patrol',
            patrolOrigin: x, patrolRange: patrolRange || 100,
            patrolDir: 1,
            alert: false,
            alertTimer: 0,
            attackTimer: 0,
            attackCooldown: 1.2,
            telegraphing: false,
            telegraphTimer: 0,
            snared: false, snareTimer: 0,
            lured: false, lureX: 0, lureTimer: 0,
            dead: false, deathTimer: 0,
            visionRange: 160,
            _hitFlash: 0
        };
        e.takeDamage = makeTakeDamage(e);
        e.die = makeDie(e);
        this.enemies.push(e);
    },

    spawnWhisperSpirit(x, y) {
        const e = {
            type: 'whisper', x, y, w: 18, h: 36,
            vx: 0, vy: 0,
            health: 25, maxHealth: 25,
            damage: 15,
            speed: 85,
            facing: 1,
            state: 'drift',
            alert: false,
            attackTimer: 0,
            attackCooldown: 1.5,
            telegraphing: false,
            telegraphTimer: 0,
            snared: false, snareTimer: 0,
            lured: false, lureX: 0, lureTimer: 0,
            dead: false, deathTimer: 0,
            driftOffset: Math.random() * Math.PI * 2,
            _hitFlash: 0
        };
        e.takeDamage = makeTakeDamage(e);
        e.die = makeDie(e);
        this.enemies.push(e);
    },

    getNearby(x, y, range) {
        let closest = null, dist = range;
        for (const e of this.enemies) {
            if (e.dead) continue;
            const d = Math.hypot(e.x - x, e.y + e.h / 2 - y);
            if (d < dist) { dist = d; closest = e; }
        }
        return closest;
    },

    update(dt) {
        for (let idx = this.enemies.length - 1; idx >= 0; idx--) {
            const e = this.enemies[idx];
            if (e.dead) {
                e.deathTimer += dt;
                if (e.deathTimer > 2) { this.enemies.splice(idx, 1); }
                continue;
            }

            e._hitFlash = Math.max(0, e._hitFlash - dt * 5);

            if (e.snared) {
                e.snareTimer -= dt;
                if (e.snareTimer <= 0) e.snared = false;
                continue;
            }

            if (e.lured) {
                e.lureTimer -= dt;
                const dir = Math.sign(e.lureX - e.x);
                e.x += dir * e.speed * dt;
                e.facing = dir;
                if (e.lureTimer <= 0 || Math.abs(e.x - e.lureX) < 10) e.lured = false;
                continue;
            }

            // Target selection (decoys)
            let targetX = Player.x, targetY = Player.y + Player.h / 2;
            let playerHidden = Player.isHidden;
            let dist = Math.hypot(targetX - e.x, targetY - (e.y + e.h / 2));

            for (const decoy of Player.decoys) {
                const dd = Math.hypot(decoy.x - e.x, (decoy.y + Player.h / 2) - (e.y + e.h / 2));
                if (dd < dist) {
                    targetX = decoy.x; targetY = decoy.y + Player.h / 2;
                    dist = dd;
                    playerHidden = false;
                }
            }

            const tdx = targetX - e.x;
            const tdist = dist;

            if (e.type === 'guard') {
                const canSee = tdist < e.visionRange && !playerHidden;

                if (e.state === 'patrol') {
                    e.x += e.patrolDir * e.speed * 0.5 * dt;
                    e.facing = e.patrolDir;
                    if (Math.abs(e.x - e.patrolOrigin) > e.patrolRange) e.patrolDir *= -1;
                    if (canSee) { e.state = 'alert'; e.alertTimer = 0.7; e.alert = true; }
                } else if (e.state === 'alert') {
                    e.alertTimer -= dt;
                    e.facing = Math.sign(tdx) || 1;
                    if (e.alertTimer <= 0) e.state = 'chase';
                    if (!canSee && tdist > e.visionRange * 1.5) { e.state = 'patrol'; e.alert = false; }
                } else if (e.state === 'chase') {
                    e.facing = Math.sign(tdx) || 1;
                    e.x += Math.sign(tdx) * e.speed * dt;
                    if (tdist < 48) {
                        e.state = 'attack';
                        e.telegraphing = true;
                        e.telegraphTimer = 0.5;
                    }
                    if (!canSee && tdist > e.visionRange * 2) { e.state = 'patrol'; e.alert = false; }
                } else if (e.state === 'attack') {
                    if (e.telegraphing) {
                        e.telegraphTimer -= dt;
                        if (e.telegraphTimer <= 0) {
                            e.telegraphing = false;
                            if (tdist < 60) Player.takeDamage(e.damage);
                            e.attackTimer = e.attackCooldown;
                        }
                    } else {
                        e.attackTimer -= dt;
                        if (e.attackTimer <= 0) e.state = 'chase';
                    }
                }

                // Gravity
                e.vy += GRAVITY;
                e.y += e.vy;
                for (const p of World.platforms) {
                    if (e.x + e.w / 2 > p.x && e.x - e.w / 2 < p.x + p.w) {
                        if (e.y + e.h >= p.y && e.y + e.h <= p.y + p.h + 12 && e.vy >= 0) {
                            e.y = p.y - e.h;
                            e.vy = 0;
                        }
                    }
                }
            } else if (e.type === 'whisper') {
                e.y += Math.sin(Game.time * 2.5 + e.driftOffset) * 0.4;

                if (tdist < 220 && !playerHidden) {
                    e.alert = true;
                    e.facing = Math.sign(tdx) || 1;
                    e.x += Math.sign(tdx) * e.speed * dt * 0.6;

                    if (tdist < 45) {
                        if (!e.telegraphing) {
                            e.attackTimer -= dt;
                            if (e.attackTimer <= 0) {
                                e.telegraphing = true;
                                e.telegraphTimer = 0.4;
                                e.attackTimer = e.attackCooldown;
                            }
                        }
                    }

                    if (e.telegraphing) {
                        e.telegraphTimer -= dt;
                        if (e.telegraphTimer <= 0) {
                            e.telegraphing = false;
                            if (tdist < 55) Player.takeDamage(e.damage);
                        }
                    }
                } else {
                    e.alert = false;
                }
            }
        }
    },

    render(ctx) {
        for (const e of this.enemies) {
            if (e.x < Camera.x - 100 || e.x > Camera.x + Game.width + 100) continue;

            if (e.dead) {
                const alpha = 1 - e.deathTimer / 2;
                if (alpha <= 0) continue;
                ctx.globalAlpha = alpha;
            }

            ctx.save();
            ctx.translate(e.x, e.y);

            if (e.type === 'guard') {
                const flash = e._hitFlash > 0;
                // Shadow
                ctx.fillStyle = 'rgba(0,0,0,0.25)';
                ctx.beginPath();
                ctx.ellipse(0, e.h, 12, 3, 0, 0, Math.PI * 2);
                ctx.fill();

                // Body
                ctx.fillStyle = flash ? '#FFF' : '#4A3A28';
                ctx.fillRect(-e.w / 2, 10, e.w, e.h - 18);
                // Armor
                ctx.fillStyle = flash ? '#FFF' : '#6B5A40';
                ctx.fillRect(-e.w / 2 + 2, 12, e.w - 4, 14);
                // Head
                ctx.fillStyle = flash ? '#FFF' : COL.skinDark;
                ctx.beginPath();
                ctx.arc(0, 6, 8, 0, Math.PI * 2);
                ctx.fill();
                // Helmet
                ctx.fillStyle = '#5A4A30';
                ctx.beginPath();
                ctx.arc(0, 3, 9, Math.PI, Math.PI * 2);
                ctx.fill();
                // Spear
                ctx.fillStyle = COL.woodDark;
                ctx.fillRect(e.facing * 8, -8, 3, 52);
                ctx.fillStyle = '#B0B0B0';
                ctx.beginPath();
                ctx.moveTo(e.facing * 8 + 1.5, -13);
                ctx.lineTo(e.facing * 8 - 3, -3);
                ctx.lineTo(e.facing * 8 + 6, -3);
                ctx.closePath();
                ctx.fill();
                // Shield
                ctx.fillStyle = '#6B5040';
                ctx.fillRect(-e.facing * 10 - 6, 14, 12, 18);
                ctx.strokeStyle = COL.gold;
                ctx.lineWidth = 0.8;
                ctx.strokeRect(-e.facing * 10 - 5, 15, 10, 16);
                // Legs
                ctx.fillStyle = flash ? '#FFF' : '#3A2A18';
                ctx.fillRect(-4, e.h - 8, 4, 8);
                ctx.fillRect(2, e.h - 8, 4, 8);

                // Telegraph
                if (e.telegraphing) {
                    const t = Math.sin(Game.time * 20) * 0.3 + 0.3;
                    ctx.fillStyle = `rgba(255,50,50,${t})`;
                    ctx.fillRect(-e.w / 2 - 5, -5, e.w + 10, e.h + 10);
                }

                // Vision cone (only patrol/alert)
                if (!e.dead && (e.state === 'patrol' || e.state === 'alert')) {
                    const vr = e.visionRange;
                    ctx.globalAlpha = e.state === 'alert' ? 0.1 : 0.06;
                    ctx.fillStyle = e.state === 'alert' ? COL.yellow : COL.green;
                    ctx.beginPath();
                    ctx.moveTo(0, e.h * 0.4);
                    ctx.lineTo(e.facing * vr, e.h * 0.4 - 45);
                    ctx.lineTo(e.facing * vr, e.h * 0.4 + 45);
                    ctx.closePath();
                    ctx.fill();
                    ctx.globalAlpha = 1;
                }

                // Snare
                if (e.snared) {
                    ctx.strokeStyle = COL.gold;
                    ctx.lineWidth = 1.5;
                    for (let i = 0; i < 5; i++) {
                        const a = (i / 5) * Math.PI * 2 + Game.time * 2;
                        ctx.beginPath();
                        ctx.moveTo(0, e.h / 2);
                        ctx.lineTo(Math.cos(a) * 20, e.h / 2 + Math.sin(a) * 20);
                        ctx.stroke();
                    }
                    ctx.beginPath();
                    ctx.arc(0, e.h / 2, 18, 0, Math.PI * 2);
                    ctx.strokeStyle = 'rgba(212,175,55,0.3)';
                    ctx.stroke();
                }

            } else if (e.type === 'whisper') {
                const drift = Math.sin(Game.time * 3 + e.driftOffset) * 3;
                ctx.globalAlpha = e.dead ? ctx.globalAlpha * 0.5 : 0.55;

                // Wispy body
                ctx.fillStyle = 'rgba(93,173,226,0.45)';
                ctx.beginPath();
                ctx.moveTo(-9 + drift, e.h);
                ctx.quadraticCurveTo(-13 + drift, e.h / 2, 0, 2);
                ctx.quadraticCurveTo(13 + drift, e.h / 2, 9 + drift, e.h);
                ctx.fill();

                // Inner glow
                ctx.fillStyle = 'rgba(180,220,255,0.35)';
                ctx.beginPath();
                ctx.ellipse(drift, e.h * 0.45, 6, 12, 0, 0, Math.PI * 2);
                ctx.fill();

                // Eyes
                ctx.fillStyle = 'rgba(255,255,255,0.75)';
                ctx.beginPath();
                ctx.arc(-3 + drift, e.h * 0.3, 2, 0, Math.PI * 2);
                ctx.arc(3 + drift, e.h * 0.3, 2, 0, Math.PI * 2);
                ctx.fill();

                // Telegraph
                if (e.telegraphing) {
                    ctx.fillStyle = `rgba(255,50,50,${Math.sin(Game.time * 20) * 0.25 + 0.25})`;
                    ctx.beginPath();
                    ctx.arc(0, e.h / 2, 22, 0, Math.PI * 2);
                    ctx.fill();
                }

                ctx.globalAlpha = 1;
            }

            ctx.restore();

            if (e.dead) ctx.globalAlpha = 1;

            // HP bar
            if (e.alert && !e.dead && e.health < e.maxHealth) {
                ctx.fillStyle = 'rgba(0,0,0,0.5)';
                ctx.fillRect(e.x - 16, e.y - 10, 32, 4);
                ctx.fillStyle = COL.red;
                ctx.fillRect(e.x - 16, e.y - 10, 32 * (e.health / e.maxHealth), 4);
            }

            // Spider Sense weak point
            if (Player.spiderSenseActive && !e.dead) {
                ctx.strokeStyle = 'rgba(212,175,55,0.6)';
                ctx.lineWidth = 1.5;
                ctx.setLineDash([3, 3]);
                ctx.beginPath();
                ctx.arc(e.x, e.y + e.h * 0.3, 8, 0, Math.PI * 2);
                ctx.stroke();
                ctx.setLineDash([]);
            }
        }
    }
};

function makeTakeDamage(e) {
    return function(amount, dir) {
        e.health -= amount;
        e._hitFlash = 1;
        if (dir) e.x += dir * 8;
        e.alert = true;
        if (e.state === 'patrol') e.state = 'chase';
        if (e.health <= 0) e.die();
    };
}

function makeDie(e) {
    return function() {
        e.dead = true;
        e.deathTimer = 0;
        Game.stats.enemies++;
        ParticleSystem.emit(e.x, e.y + e.h / 2, 12, {
            color: e.type === 'whisper' ? COL.spiritBlue : COL.gold,
            life: 0.5, size: 3, speed: 100, gravity: 0.3
        });
    };
}

// ============ BOSS MANAGER ============
const BossManager = {
    active: false,
    defeated: false,
    boss: null,
    phase: 1,
    phaseTimer: 0,
    illusions: [],
    _defeatTimeout: null,

    start() {
        this.active = true;
        this.phase = 1;
        this.boss = {
            x: AREAS.bossArena.x + 600, y: 620,
            w: 50, h: 70,
            vx: 0, vy: 0,
            health: 180, maxHealth: 180,
            facing: -1,
            state: 'idle',
            attackTimer: 2,
            dead: false,
            _hitFlash: 0,
            invuln: false
        };
        this.illusions = [];

        document.getElementById('boss-bar').style.display = 'block';
        Objective.set('Defeat the Mask Guardian');
        Audio.startBossMusic();
        HUD.showAreaName('The Mask Guardian', 'Phase 1 - Ceremonial Wrath');
        Game.shake(6, 0.5);
    },

    takeDamage(amount) {
        if (!this.boss || this.boss.dead || this.boss.invuln) return;
        this.boss.health -= amount;
        this.boss._hitFlash = 1;

        document.getElementById('boss-fill').style.width = (this.boss.health / this.boss.maxHealth * 100) + '%';

        ParticleSystem.emit(this.boss.x, this.boss.y + 30, 8, {
            color: COL.corruption, life: 0.4, size: 4, speed: 100
        });

        if (this.boss.health <= this.boss.maxHealth * 0.6 && this.phase === 1) {
            this.phase = 2;
            this.boss.invuln = true;
            this.phaseTimer = 0;
            this.createIllusions();
            HUD.showAreaName('Phase 2', 'Find the real mask - use Spider Sense');
            Game.shake(8, 0.6);
            setTimeout(() => { if (this.boss) this.boss.invuln = false; }, 2000);
        } else if (this.boss.health <= this.boss.maxHealth * 0.25 && this.phase === 2) {
            this.phase = 3;
            this.illusions = [];
            this.boss.invuln = true;
            this.phaseTimer = 0;
            HUD.showAreaName('Phase 3', 'Enraged — dodge and strike!');
            Game.shake(10, 0.8);
            ParticleSystem.emit(this.boss.x, this.boss.y + 35, 30, {
                color: COL.corruption, life: 1, size: 5, speed: 150, fadeOut: true
            });
            setTimeout(() => { if (this.boss) this.boss.invuln = false; }, 2000);
        }

        if (this.boss.health <= 0) {
            this.boss.health = 0;
            this.boss.dead = true;
            this.defeat();
        }
    },

    createIllusions() {
        for (let i = 0; i < 3; i++) {
            this.illusions.push({
                x: AREAS.bossArena.x + 200 + i * 300,
                y: 620, w: 50, h: 70,
                facing: Math.random() > 0.5 ? 1 : -1,
                health: 30,
                _hitFlash: 0
            });
        }
    },

    defeat() {
        this.active = false;
        this.defeated = true;
        document.getElementById('boss-bar').style.display = 'none';

        Game.stats.fragments++;
        Player.abilities.trickMirror = true;
        document.getElementById('ability-f').setAttribute('data-locked', 'false');

        Game.triggerSlowMo(0.15, 1.5);
        Game.shake(12, 1);

        for (let i = 0; i < 5; i++) {
            setTimeout(() => {
                if (this.boss) {
                    ParticleSystem.emit(this.boss.x, this.boss.y + 35, 30, {
                        color: COL.goldBright, life: 1.5, size: 5, speed: 200, gravity: 0.3, fadeOut: true
                    });
                }
            }, i * 200);
        }

        // Remove right arena wall so player can proceed
        const arenaRightX = AREAS.bossArena.x + AREAS.bossArena.w - 10;
        World.walls = World.walls.filter(w => !(w.x === arenaRightX && w.y === 200));

        setTimeout(() => {
            Objective.set('The path opens. Continue east.');
        }, 2500);
    },

    update(dt) {
        if (!this.active || !this.boss || this.boss.dead) return;

        this.boss._hitFlash = Math.max(0, this.boss._hitFlash - dt * 5);
        this.phaseTimer += dt;

        const b = this.boss;
        b.facing = Math.sign(Player.x - b.x) || 1;

        if (this.phase === 1) {
            b.attackTimer -= dt;
            if (b.attackTimer <= 0) {
                const roll = Math.random();
                if (roll < 0.5) {
                    // Sweep
                    b.state = 'sweep';
                    b.attackTimer = 2.2;
                    // Telegraph then damage
                    setTimeout(() => {
                        if (!b.dead && Math.abs(Player.x - b.x) < 130 && Math.abs(Player.y - b.y) < 80) {
                            Player.takeDamage(18);
                        }
                        ParticleSystem.emit(b.x, b.y + 50, 15, {
                            color: COL.corruption, life: 0.3, size: 5, speed: 150
                        });
                        Game.shake(5, 0.3);
                        b.state = 'idle';
                    }, 500);
                } else {
                    // Charge toward player
                    b.state = 'charge';
                    b.attackTimer = 2.8;
                    b.vx = b.facing * 220;
                    setTimeout(() => {
                        b.vx = 0;
                        b.state = 'idle';
                        if (Math.abs(Player.x - b.x) < 50) Player.takeDamage(15);
                    }, 700);
                }
            }
            b.x += b.vx * dt;
            b.x = clamp(b.x, AREAS.bossArena.x + 60, AREAS.bossArena.x + AREAS.bossArena.w - 60);

        } else if (this.phase === 2) {
            b.attackTimer -= dt;
            b.x += Math.sin(Game.time * 1.2) * 60 * dt;
            b.x = clamp(b.x, AREAS.bossArena.x + 60, AREAS.bossArena.x + AREAS.bossArena.w - 60);

            if (b.attackTimer <= 0) {
                b.attackTimer = 1.8;
                if (Math.abs(Player.x - b.x) < 90) Player.takeDamage(14);
                ParticleSystem.emit(b.x, b.y + 30, 8, {
                    color: COL.corruption, life: 0.3, size: 4, speed: 80
                });
            }

            for (const ill of this.illusions) {
                ill._hitFlash = Math.max(0, ill._hitFlash - dt * 5);
                ill.x += Math.sin(Game.time * 0.8 + ill.x * 0.01) * 30 * dt;
            }

        } else if (this.phase === 3) {
            // Enraged — faster, more aggressive, stays on ground
            b.attackTimer -= dt;
            if (b.attackTimer <= 0) {
                const roll = Math.random();
                if (roll < 0.4) {
                    // Fast charge
                    b.state = 'charge';
                    b.attackTimer = 1.5;
                    b.vx = b.facing * 350;
                    setTimeout(() => {
                        b.vx = 0;
                        b.state = 'idle';
                        if (Math.abs(Player.x - b.x) < 60) Player.takeDamage(20);
                        Game.shake(6, 0.4);
                    }, 500);
                } else if (roll < 0.7) {
                    // Sweep
                    b.state = 'sweep';
                    b.attackTimer = 1.2;
                    setTimeout(() => {
                        if (!b.dead && Math.abs(Player.x - b.x) < 140) Player.takeDamage(22);
                        ParticleSystem.emit(b.x, b.y + 50, 20, {
                            color: COL.corruption, life: 0.3, size: 6, speed: 180
                        });
                        Game.shake(7, 0.4);
                        b.state = 'idle';
                    }, 400);
                } else {
                    // Spawn minion
                    b.attackTimer = 2.0;
                    EnemyManager.spawnWhisperSpirit(
                        AREAS.bossArena.x + 200 + Math.random() * 800, 600
                    );
                }
            }
            b.x += b.vx * dt;
            b.x = clamp(b.x, AREAS.bossArena.x + 60, AREAS.bossArena.x + AREAS.bossArena.w - 60);
        }

        // Gravity (all phases)
        b.vy += GRAVITY * 0.5;
        b.y += b.vy;
        if (b.y + b.h > 700) { b.y = 700 - b.h; b.vy = 0; }
    },

    render(ctx) {
        if (!this.boss) return;
        if (!this.active && !this.defeated) return;

        // Illusions
        for (const ill of this.illusions) {
            this.renderMask(ctx, ill, false);
        }

        // Boss
        if (!this.boss.dead || this.boss._hitFlash > 0) {
            this.renderMask(ctx, this.boss, true);
        }
    },

    renderMask(ctx, m, isReal) {
        ctx.save();
        ctx.translate(m.x, m.y);

        const flash = m._hitFlash > 0;
        const float = Math.sin(Game.time * 2 + m.x * 0.01) * 5;

        // Robes
        ctx.fillStyle = flash ? 'rgba(255,255,255,0.6)' : 'rgba(80,30,150,0.5)';
        ctx.beginPath();
        ctx.moveTo(-25, 35 + float);
        ctx.quadraticCurveTo(-30, m.h + float, 0, m.h + 10 + float);
        ctx.quadraticCurveTo(30, m.h + float, 25, 35 + float);
        ctx.fill();

        // Mask
        ctx.fillStyle = flash ? '#FFF' : '#C0A050';
        ctx.beginPath();
        ctx.ellipse(0, 18 + float, 22, 28, 0, 0, Math.PI * 2);
        ctx.fill();
        ctx.strokeStyle = COL.gold;
        ctx.lineWidth = 2;
        ctx.stroke();

        // Eyes
        const eg = Math.sin(Game.time * 4) * 0.3 + 0.7;
        ctx.fillStyle = `rgba(160,50,200,${eg})`;
        ctx.beginPath();
        ctx.ellipse(-8, 10 + float, 6, 4, -0.2, 0, Math.PI * 2);
        ctx.fill();
        ctx.beginPath();
        ctx.ellipse(8, 10 + float, 6, 4, 0.2, 0, Math.PI * 2);
        ctx.fill();

        // Mouth
        ctx.strokeStyle = 'rgba(100,30,150,0.7)';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(0, 30 + float, 10, 0.3, Math.PI - 0.3);
        ctx.stroke();

        // Horns
        ctx.fillStyle = flash ? '#FFF' : '#8B6914';
        ctx.beginPath();
        ctx.moveTo(-15, -2 + float);
        ctx.lineTo(-22, -28 + float);
        ctx.lineTo(-8, -5 + float);
        ctx.fill();
        ctx.beginPath();
        ctx.moveTo(15, -2 + float);
        ctx.lineTo(22, -28 + float);
        ctx.lineTo(8, -5 + float);
        ctx.fill();

        // Spider Sense reveals real
        if (isReal && Player.spiderSenseActive) {
            ctx.strokeStyle = 'rgba(212,175,55,0.8)';
            ctx.lineWidth = 2;
            ctx.setLineDash([4, 4]);
            ctx.beginPath();
            ctx.arc(0, m.h / 2 + float, 42, 0, Math.PI * 2);
            ctx.stroke();
            ctx.setLineDash([]);
            // "REAL" text
            ctx.font = '10px Rajdhani, sans-serif';
            ctx.fillStyle = COL.gold;
            ctx.textAlign = 'center';
            ctx.fillText('REAL', 0, m.h / 2 + float + 55);
        }

        // Sweep telegraph
        if (isReal && this.boss && this.boss.state === 'sweep') {
            const t = Math.sin(Game.time * 15) * 0.3 + 0.3;
            ctx.fillStyle = `rgba(255,50,50,${t})`;
            ctx.fillRect(-65, m.h / 2 - 12 + float, 130, 24);
        }

        ctx.restore();
    }
};

// ============ PARTICLE SYSTEM ============
const ParticleSystem = {
    particles: [],

    init() { this.particles = []; },

    emit(x, y, count, opts) {
        for (let i = 0; i < count; i++) {
            const angle = Math.random() * Math.PI * 2;
            const speed = (opts.speed || 50) * (0.5 + Math.random() * 0.5);
            this.particles.push({
                x, y,
                vx: Math.cos(angle) * speed,
                vy: Math.sin(angle) * speed - speed * 0.3,
                life: (opts.life || 0.5) * (0.7 + Math.random() * 0.3),
                maxLife: opts.life || 0.5,
                size: (opts.size || 3) * (0.7 + Math.random() * 0.3),
                color: opts.color || '#FFF',
                gravity: opts.gravity || 0,
                fadeOut: opts.fadeOut !== false
            });
        }
    },

    update(dt) {
        for (let i = this.particles.length - 1; i >= 0; i--) {
            const p = this.particles[i];
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.vy += (p.gravity || 0) * 60 * dt;
            p.vx *= 0.99;
            p.life -= dt;
            if (p.life <= 0) this.particles.splice(i, 1);
        }
        if (this.particles.length > 400) this.particles.splice(0, this.particles.length - 400);
    },

    render(ctx) {
        for (const p of this.particles) {
            const alpha = p.fadeOut ? Math.max(0, p.life / p.maxLife) : 1;
            ctx.globalAlpha = alpha;
            ctx.fillStyle = p.color;
            const s = p.size * (p.fadeOut ? (0.5 + 0.5 * p.life / p.maxLife) : 1);
            ctx.fillRect(p.x - s / 2, p.y - s / 2, s, s);
        }
        ctx.globalAlpha = 1;
    }
};

// ============ DIALOGUE SYSTEM ============
const Dialogue = {
    active: false,
    lines: [],
    currentLine: 0,
    charIndex: 0,
    charTimer: 0,
    charSpeed: 0.025,
    fullText: '',
    choices: null,
    callback: null,
    _keyHandlers: [],

    init() {},

    start(lines, name, color, callback) {
        this.active = true;
        this.lines = lines;
        this.currentLine = 0;
        this.callback = callback || null;
        Game.state = 'dialogue';
        this.showLine(0);
        document.getElementById('dialogue-box').classList.add('active');
    },

    showLine(index) {
        // Clean up key handlers
        for (const h of this._keyHandlers) window.removeEventListener('keydown', h);
        this._keyHandlers = [];

        if (index >= this.lines.length) {
            this.end();
            return;
        }
        const line = this.lines[index];
        this.currentLine = index;
        this.fullText = line.text;
        this.charIndex = 0;
        this.charTimer = 0;
        this.choices = line.choices || null;

        document.getElementById('dialogue-speaker').textContent = line.speaker || '';
        document.getElementById('dialogue-text').textContent = '';
        document.getElementById('dialogue-continue').style.display = this.choices ? 'none' : 'block';

        const portrait = document.getElementById('dialogue-portrait');
        if (line.speaker === 'Kweku') {
            portrait.style.background = `linear-gradient(135deg, ${COL.cloth1}, ${COL.skinWarm})`;
        } else if (line.speaker === 'Anansi') {
            portrait.style.background = `linear-gradient(135deg, ${COL.goldBright}, #1A0A2E)`;
        } else if (line.speaker === 'Ama Serwaa') {
            portrait.style.background = `linear-gradient(135deg, ${COL.cloth3}, ${COL.skinDark})`;
        } else {
            portrait.style.background = 'rgba(30,20,45,0.8)';
        }

        document.getElementById('dialogue-choices').innerHTML = '';
    },

    update(dt) {
        if (!this.active) return;

        if (this.charIndex < this.fullText.length) {
            this.charTimer += dt;
            while (this.charTimer >= this.charSpeed && this.charIndex < this.fullText.length) {
                this.charTimer -= this.charSpeed;
                this.charIndex++;
            }
            document.getElementById('dialogue-text').textContent = this.fullText.substring(0, this.charIndex);

            // Skip ahead
            if (Input.wasPressed('Space') || Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp')) {
                this.charIndex = this.fullText.length;
                document.getElementById('dialogue-text').textContent = this.fullText;
                if (this.choices) this.showChoices();
            }
        } else {
            if (this.choices && document.getElementById('dialogue-choices').children.length === 0) {
                this.showChoices();
            }
            if (!this.choices && (Input.wasPressed('Space') || Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp'))) {
                this.showLine(this.currentLine + 1);
            }
        }
    },

    showChoices() {
        const container = document.getElementById('dialogue-choices');
        container.innerHTML = '';
        document.getElementById('dialogue-continue').style.display = 'none';

        this.choices.forEach((choice, i) => {
            const el = document.createElement('div');
            el.className = 'choice';
            el.textContent = `[${i + 1}] ${choice.text}`;
            el.onclick = () => this.selectChoice(choice);
            container.appendChild(el);
        });

        // Keyboard handlers
        this.choices.forEach((choice, i) => {
            const handler = (e) => {
                if (e.key === String(i + 1)) {
                    this.selectChoice(choice);
                }
            };
            window.addEventListener('keydown', handler);
            this._keyHandlers.push(handler);
        });
    },

    selectChoice(choice) {
        for (const h of this._keyHandlers) window.removeEventListener('keydown', h);
        this._keyHandlers = [];
        this.choices = null;
        if (choice.next !== undefined) {
            this.showLine(choice.next);
        } else {
            this.end();
        }
    },

    end() {
        this.active = false;
        Game.state = 'playing';
        document.getElementById('dialogue-box').classList.remove('active');
        document.getElementById('dialogue-choices').innerHTML = '';
        for (const h of this._keyHandlers) window.removeEventListener('keydown', h);
        this._keyHandlers = [];
        if (this.callback) {
            const cb = this.callback;
            this.callback = null;
            cb();
        }
    }
};

// ============ PUZZLE SYSTEM ============
const Puzzle = {
    init() {},
    started: false,
    solved: false,
    symbols: ['gye_nyame', 'sankofa', 'dwennimmen'],
    correctOrder: [1, 2, 0], // sankofa, dwennimmen, gye_nyame
    playerOrder: [],
    wrongAttempts: 0,
    symbolPositions: [],
    glowTimers: [0, 0, 0],

    start() {
        if (this.started) return;
        this.started = true;
        Objective.set('Solve the Proverb Gate: activate Adinkra symbols in the correct order');

        const gx = AREAS.proverbGate.x + 200;
        this.symbolPositions = [
            { x: gx, y: 670, symbol: 0, activated: false },
            { x: gx + 100, y: 670, symbol: 1, activated: false },
            { x: gx + 200, y: 670, symbol: 2, activated: false }
        ];

        Dialogue.start([
            { speaker: 'Kweku', text: "A locked gate with Adinkra symbols. The drummer said something about three truths..." },
            { speaker: 'Kweku', text: "'The return of the bird, the wisdom of the ram, and the eye of God.' Sankofa, Dwennimmen, Gye Nyame. I need to activate them in that order." },
            { speaker: 'Kweku', text: "(Use Spider Sense [R] to see the symbols. Walk near each and press [W] to activate. Order matters!)" }
        ], 'Kweku', COL.cloth1);
    },

    update(dt) {
        if (!this.started || this.solved) return;

        for (let i = 0; i < 3; i++) {
            if (this.glowTimers[i] > 0) this.glowTimers[i] -= dt;
        }

        if (Game.state === 'dialogue') return;

        if (Input.wasPressed('KeyW') || Input.wasPressed('ArrowUp')) {
            for (let i = 0; i < this.symbolPositions.length; i++) {
                const sp = this.symbolPositions[i];
                if (sp.activated) continue;
                if (Math.abs(Player.x - sp.x) < 45 && Math.abs(Player.y + Player.h - sp.y) < 60) {
                    this.activateSymbol(i);
                    break;
                }
            }
        }
    },

    activateSymbol(index) {
        const sp = this.symbolPositions[index];
        this.playerOrder.push(sp.symbol);
        sp.activated = true;
        this.glowTimers[index] = 1;
        Audio.playPuzzle();

        ParticleSystem.emit(sp.x, sp.y, 10, {
            color: COL.gold, life: 0.6, size: 3, speed: 60
        });

        if (this.playerOrder.length === 3) {
            const correct = this.playerOrder.every((v, i) => v === this.correctOrder[i]);
            if (correct) this.solve();
            else this.fail();
        }
    },

    solve() {
        this.solved = true;
        Game.state = 'playing';
        Game.stats.fragments++;
        Objective.set('The gate opens! Proceed to the Mask Chamber.');
        Audio.playPuzzleSolve();
        Game.shake(6, 0.5);

        // Remove gate wall
        World.walls = World.walls.filter(w =>
            !(w.x >= AREAS.proverbGate.x + 600 && w.x <= AREAS.proverbGate.x + 800));

        const gx = AREAS.proverbGate.x + 400;
        for (let i = 0; i < 4; i++) {
            setTimeout(() => {
                ParticleSystem.emit(gx, 350 + i * 80, 20, {
                    color: COL.goldBright, life: 1.2, size: 4, speed: 130, gravity: 0.15, fadeOut: true
                });
            }, i * 250);
        }
    },

    fail() {
        this.wrongAttempts++;
        this.playerOrder = [];
        for (const sp of this.symbolPositions) sp.activated = false;

        Game.shake(4, 0.3);
        Audio.playPuzzleFail();
        Objective.set('Wrong order! Remember: bird, ram, God.');

        if (this.wrongAttempts <= 3) {
            EnemyManager.spawnWhisperSpirit(
                AREAS.proverbGate.x + 80 + Math.random() * 200, 600
            );
        }
    },

    render(ctx) {
        if (!this.started) return;

        for (let i = 0; i < this.symbolPositions.length; i++) {
            const sp = this.symbolPositions[i];
            const glowing = this.glowTimers[i] > 0 || sp.activated;

            // Circle bg
            ctx.fillStyle = glowing ? 'rgba(212,175,55,0.2)' : 'rgba(40,30,25,0.6)';
            ctx.beginPath();
            ctx.arc(sp.x, sp.y, 28, 0, Math.PI * 2);
            ctx.fill();
            ctx.strokeStyle = glowing ? COL.gold : 'rgba(212,175,55,0.25)';
            ctx.lineWidth = glowing ? 2.5 : 1.5;
            ctx.stroke();

            // Symbol
            World.drawAdinkra(ctx, sp.x, sp.y, this.symbols[sp.symbol], 22);

            // Label (Spider Sense)
            if (Player.spiderSenseActive) {
                ctx.font = '12px Rajdhani, sans-serif';
                ctx.fillStyle = COL.gold;
                ctx.textAlign = 'center';
                ctx.fillText(['Gye Nyame', 'Sankofa', 'Dwennimmen'][sp.symbol], sp.x, sp.y + 42);
            }

            // Interaction hint
            if (!sp.activated && Math.abs(Player.x - sp.x) < 45 && Math.abs(Player.y + Player.h - sp.y) < 60) {
                ctx.font = '11px Rajdhani, sans-serif';
                ctx.fillStyle = 'rgba(212,175,55,0.7)';
                ctx.textAlign = 'center';
                ctx.fillText('[W] Activate', sp.x, sp.y - 35);
            }
        }
    }
};

// ============ CINEMATIC SYSTEM ============
const Cinematic = {
    active: false,
    lockPlayer: false,
    fadeAlpha: 0,
    fadeTarget: 0,
    _storyTriggered: false,
    _endingStarted: false,
    _checkStoryteller: null,

    init() {},

    startFestival() {
        this.active = true;
        this.lockPlayer = true;
        Game.state = 'cutscene';
        Player.x = AREAS.festival.x + 100;
        Player.y = 600;

        this.fadeAlpha = 1;
        this.fadeTarget = 0;

        HUD.showAreaName('The Festival Grounds', 'Kingdom of Asanteman');
        Audio.startFestivalMusic();

        setTimeout(() => {
            Objective.set('Explore the festival. Talk to the people. Head east to the stage.');
            this.lockPlayer = false;
            Game.state = 'playing';
            this.active = false;
            this.setupStorytellerTrigger();
        }, 2500);
    },

    setupStorytellerTrigger() {
        this._checkStoryteller = setInterval(() => {
            if (Player.x > AREAS.festival.x + 1000 && !this._storyTriggered) {
                this._storyTriggered = true;
                clearInterval(this._checkStoryteller);
                this.triggerMurder();
            }
        }, 300);
    },

    triggerMurder() {
        Dialogue.start([
            { speaker: 'Storyteller', text: "Gather, children of Asanteman! Tonight I weave a tale never told... a fragment of the First Story, hidden by Anansi himself at the dawn of all things..." },
            { speaker: 'Storyteller', text: "In the time before time, when the sky was close enough to touch, there lived a spider who knew every story ever told, except one--" },
            { speaker: '???', text: "The story... must not... be told..." },
            { speaker: 'Kweku', text: "What-- The storyteller! Something struck him! The sky... what's happening to the SKY?!" }
        ], 'Storyteller', '#FFF', () => {
            this.spiritBreach();
        });
    },

    spiritBreach() {
        World.spiritBreached = true;
        Game.shake(15, 1.5);
        Audio.startBreachSound();

        this.fadeAlpha = 0.8;
        this.fadeTarget = 0;

        Objective.set('ESCAPE! Head east through the rooftops!');

        // Spawn city enemies
        EnemyManager.spawnGuard(AREAS.cityEscape.x + 300, 600, 150);
        EnemyManager.spawnGuard(AREAS.cityEscape.x + 700, 600, 200);
        EnemyManager.spawnGuard(AREAS.cityEscape.x + 1100, 600, 150);
        EnemyManager.spawnWhisperSpirit(AREAS.cityEscape.x + 500, 480);
        EnemyManager.spawnWhisperSpirit(AREAS.cityEscape.x + 900, 430);

        Game.stats.fragments++;
    },

    startShrine() {
        Game.state = 'cutscene';
        this.lockPlayer = true;

        const sx = AREAS.shrine.x + 400;
        for (let i = 0; i < 25; i++) {
            setTimeout(() => {
                ParticleSystem.emit(sx, 420, 4, {
                    color: COL.goldBright, life: 1.5, size: 2, speed: 70, gravity: -0.15, fadeOut: true
                });
            }, i * 80);
        }

        Audio.playAnansiAppear();

        setTimeout(() => {
            Dialogue.start([
                { speaker: 'Anansi', text: "Eh-heh... so the little apprentice griot finds my shrine. How... delightful." },
                { speaker: 'Kweku', text: "Who are you? I can see golden threads everywhere..." },
                { speaker: 'Anansi', text: "I am the Spider. The Keeper of Stories. The one who tricked Death and stole wisdom from the Sky God. And you, little Kweku, have stumbled into my web." },
                { speaker: 'Kweku', text: "(This can't be real...)", choices: [
                    { text: "You caused the breach. The storyteller is dead because of you.", next: 4 },
                    { text: "If you're really Anansi... can you fix this?", next: 5 }
                ]},
                { speaker: 'Anansi', text: "Me? Oh no, no, no. The fool opened a door he shouldn't have. I merely... left it unlocked. But the breach threatens my stories too. So I will give you a gift. An investment." },
                { speaker: 'Anansi', text: "Fix it? Hmm. Perhaps. But I cannot weave alone. I need clever hands. YOUR hands. Take my mark, and I will give you the threads to mend what is broken." },
                { speaker: 'Anansi', text: "There. The Mark of the Spider. You can now walk my Silken Path and I've opened your Spider Sense. Use them wisely. Or don't. Entertainment is entertainment." },
                { speaker: 'Kweku', text: "My hand... it burns. But I can feel it all. The threads of the world itself." },
                { speaker: 'Anansi', text: "Yes, yes. Very dramatic. Now go. The Corrupted Quarter lies ahead. Find the story fragments before the spirit world devours everything. And Kweku? Do try to be clever." }
            ], 'Anansi', COL.gold, () => {
                Player.abilities.silkenPath = true;
                Player.abilities.spiderSense = true;
                Player.abilities.borrowedVoice = true;
                Player.spiderMark = true;
                document.getElementById('ability-e').setAttribute('data-locked', 'false');
                document.getElementById('ability-r').setAttribute('data-locked', 'false');
                document.getElementById('ability-v').setAttribute('data-locked', 'false');

                Game.state = 'playing';
                this.lockPlayer = false;
                Objective.set('Enter the Corrupted Quarter. New abilities: Silken Path [E], Spider Sense [R], Borrowed Voice [V]');

                EnemyManager.spawnGuard(AREAS.corrupted.x + 200, 600, 150);
                EnemyManager.spawnGuard(AREAS.corrupted.x + 600, 600, 200);
                EnemyManager.spawnGuard(AREAS.corrupted.x + 1000, 600, 150);
                EnemyManager.spawnGuard(AREAS.corrupted.x + 1400, 600, 100);
                EnemyManager.spawnWhisperSpirit(AREAS.corrupted.x + 400, 450);
                EnemyManager.spawnWhisperSpirit(AREAS.corrupted.x + 800, 400);
                EnemyManager.spawnWhisperSpirit(AREAS.corrupted.x + 1200, 480);

                Audio.startCorruptedMusic();
            });
        }, 1800);
    },

    startEnding() {
        Player.x = AREAS.ending.x + 200;
        Player.y = 600;

        setTimeout(() => {
            Dialogue.start([
                { speaker: '???', text: "Stop right there." },
                { speaker: 'Kweku', text: "Who-- an arrow aimed at my face? I'm not your enemy!" },
                { speaker: 'Ama Serwaa', text: "I am Ama Serwaa, hunter-priestess of the Eastern Shrine. You carry the Spider's mark. That makes you either a fool or a threat." },
                { speaker: 'Kweku', text: "(She doesn't trust me...)", choices: [
                    { text: "I'm trying to fix the breach. Anansi gave me the tools.", next: 4 },
                    { text: "And you carry a bow aimed at my face. That makes you rude.", next: 5 }
                ]},
                { speaker: 'Ama Serwaa', text: "Anansi gives nothing freely. Everything he offers has a price. I've tracked the breach from the eastern lands. Perhaps we can work together. For now." },
                { speaker: 'Ama Serwaa', text: "...Hmph. You have spirit, at least. The spider would not choose a coward. Very well. But I am watching you, and I am watching HIM." },
                { speaker: 'Anansi', text: "(whispering) Oh, I like her. This will be entertaining..." },
                { speaker: 'Kweku', text: "The story is far from over. But at least now... I'm not alone." }
            ], 'Ama Serwaa', COL.cloth3, () => {
                this.showDemoComplete();
            });
        }, 500);
    },

    showDemoComplete() {
        Game.state = 'demo_complete';
        document.getElementById('hud').style.display = 'none';
        Audio.stopMusic();

        document.getElementById('stat-enemies').textContent = Game.stats.enemies;
        document.getElementById('stat-fragments').textContent = Game.stats.fragments;
        document.getElementById('stat-stealth').textContent = Game.stats.stealth;
        document.getElementById('stat-combo').textContent = Game.stats.maxCombo;

        document.getElementById('demo-complete-screen').style.display = 'flex';
    },

    update(dt) {
        if (Math.abs(this.fadeAlpha - this.fadeTarget) > 0.005) {
            this.fadeAlpha += (this.fadeTarget - this.fadeAlpha) * 0.04;
        } else {
            this.fadeAlpha = this.fadeTarget;
        }
    },

    render(ctx) {
        if (this.fadeAlpha > 0.005) {
            ctx.fillStyle = `rgba(0,0,0,${this.fadeAlpha})`;
            ctx.fillRect(0, 0, Game.width, Game.height);
        }
    }
};

// ============ HUD ============
const HUD = {
    areaNameTimer: 0,

    update(dt) {
        document.getElementById('health-fill').style.width = (Player.health / Player.maxHealth * 100) + '%';
        document.getElementById('energy-fill').style.width = (Player.energy / Player.maxEnergy * 100) + '%';
        document.getElementById('fragment-count').textContent = `${Game.stats.fragments} / 3`;

        // Combo
        const combo = document.getElementById('combo-container');
        if (Player.comboCount > 1) {
            combo.classList.add('active');
            document.getElementById('combo-count').textContent = Player.comboCount;
            let rating = 'D';
            if (Player.comboCount >= 20) rating = 'SSS';
            else if (Player.comboCount >= 15) rating = 'SS';
            else if (Player.comboCount >= 10) rating = 'S';
            else if (Player.comboCount >= 7) rating = 'A';
            else if (Player.comboCount >= 5) rating = 'B';
            else if (Player.comboCount >= 3) rating = 'C';
            document.getElementById('style-rating').textContent = rating;
        } else {
            combo.classList.remove('active');
        }

        if (this.areaNameTimer > 0) {
            this.areaNameTimer -= dt;
            if (this.areaNameTimer <= 0) {
                document.getElementById('area-name-display').classList.remove('visible');
            }
        }

        this.updateAbility('ability-e', Player.cooldowns.e, 2);
        this.updateAbility('ability-r', Player.cooldowns.r, 5);
        this.updateAbility('ability-f', Player.cooldowns.f, 8);
        this.updateAbility('ability-v', Player.cooldowns.v, 4);

        // Interaction prompt
        const nearNPC = NPCManager.getNearby(Player.x, Player.y + Player.h / 2, 60);
        const prompt = document.getElementById('interact-prompt');
        if (nearNPC && Game.state === 'playing') {
            prompt.classList.add('active');
            document.getElementById('prompt-text').textContent = `Talk to ${nearNPC.name}`;
        } else {
            prompt.classList.remove('active');
        }
    },

    updateAbility(id, cooldown, maxCooldown) {
        const el = document.getElementById(id);
        if (!el) return;
        const overlay = el.querySelector('.cooldown-overlay');
        if (cooldown > 0) {
            overlay.style.height = (cooldown / maxCooldown * 100) + '%';
        } else {
            overlay.style.height = '0%';
        }
    },

    showAreaName(name, sub) {
        document.getElementById('area-name-text').textContent = name;
        document.getElementById('area-name-sub').textContent = sub || '';
        this.areaNameTimer = 3.5;
        document.getElementById('area-name-display').classList.add('visible');
    }
};

// ============ OBJECTIVE ============
const Objective = {
    text: '',
    timer: 0,

    set(text) {
        this.text = text;
        this.timer = 10;
        const el = document.getElementById('objective-text');
        el.textContent = text;
        el.classList.add('visible');
    },

    update(dt) {
        if (this.timer > 0) {
            this.timer -= dt;
            if (this.timer <= 0) {
                document.getElementById('objective-text').classList.remove('visible');
            }
        }
    }
};

// ============ AUDIO ============
const Audio = {
    ctx: null,
    masterGain: null,
    musicGain: null,
    sfxGain: null,
    drumInterval: null,
    initialized: false,

    init() {
        try {
            this.ctx = new (window.AudioContext || window.webkitAudioContext)();
            this.masterGain = this.ctx.createGain();
            this.masterGain.gain.value = 0.25;
            this.masterGain.connect(this.ctx.destination);
            this.musicGain = this.ctx.createGain();
            this.musicGain.gain.value = 0.35;
            this.musicGain.connect(this.masterGain);
            this.sfxGain = this.ctx.createGain();
            this.sfxGain.gain.value = 0.5;
            this.sfxGain.connect(this.masterGain);
            this.initialized = true;
        } catch (e) {
            console.warn('Audio not available');
        }
    },

    update(dt) {},

    startFestivalMusic() {
        if (!this.initialized) return;
        this.stopMusic();
        let beat = 0;
        const pattern = [1, 0, 0.6, 0, 1, 0.6, 0, 0.4, 1, 0, 0.5, 0.4, 1, 0, 0.6, 0];
        const interval = 60000 / 120 / 4;
        this.drumInterval = setInterval(() => {
            const vol = pattern[beat % pattern.length];
            if (vol > 0) this.playDrum(vol * 0.25, beat % 4 === 0 ? 80 : 130);
            if (beat % 3 === 0) this.playDrum(0.08, 210);
            if (beat % 16 === 0) this.playTone(440, 0.04, 1.5, 'sine');
            if (beat % 16 === 8) this.playTone(523, 0.03, 1, 'sine');
            beat++;
        }, interval);
    },

    startBreachSound() {
        if (!this.initialized) return;
        this.stopMusic();
        this.playTone(55, 0.08, 3, 'sawtooth');
        this.playTone(58, 0.06, 2.5, 'sawtooth');
        this.playNoise(0.15, 2);
    },

    startCorruptedMusic() {
        if (!this.initialized) return;
        this.stopMusic();
        let beat = 0;
        const interval = 60000 / 85 / 4;
        this.drumInterval = setInterval(() => {
            if (beat % 4 === 0) this.playDrum(0.15, 55);
            if (beat % 8 === 0) this.playTone(200 + Math.random() * 60, 0.025, 0.6, 'sine');
            if (beat % 6 === 0) this.playNoise(0.015, 0.25);
            beat++;
        }, interval);
    },

    startBossMusic() {
        if (!this.initialized) return;
        this.stopMusic();
        let beat = 0;
        const interval = 60000 / 140 / 4;
        this.drumInterval = setInterval(() => {
            const accent = beat % 4 === 0;
            this.playDrum(accent ? 0.28 : 0.12, accent ? 55 : 100);
            if (beat % 2 === 0) this.playDrum(0.08, 190);
            if (beat % 8 === 0) this.playTone(110, 0.04, 0.3, 'sawtooth');
            beat++;
        }, interval);
    },

    stopMusic() {
        if (this.drumInterval) { clearInterval(this.drumInterval); this.drumInterval = null; }
    },

    playDrum(vol, freq) {
        if (!this.initialized) return;
        const t = this.ctx.currentTime;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = 'triangle';
        osc.frequency.setValueAtTime(freq, t);
        osc.frequency.exponentialRampToValueAtTime(Math.max(freq * 0.4, 20), t + 0.1);
        gain.gain.setValueAtTime(vol, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + 0.15);
        osc.connect(gain);
        gain.connect(this.musicGain);
        osc.start(t);
        osc.stop(t + 0.2);
    },

    playTone(freq, vol, duration, type) {
        if (!this.initialized) return;
        const t = this.ctx.currentTime;
        const osc = this.ctx.createOscillator();
        const gain = this.ctx.createGain();
        osc.type = type || 'sine';
        osc.frequency.setValueAtTime(freq, t);
        gain.gain.setValueAtTime(vol, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + duration);
        osc.connect(gain);
        gain.connect(this.musicGain);
        osc.start(t);
        osc.stop(t + duration + 0.05);
    },

    playNoise(vol, duration) {
        if (!this.initialized) return;
        const t = this.ctx.currentTime;
        const len = Math.floor(this.ctx.sampleRate * duration);
        const buffer = this.ctx.createBuffer(1, len, this.ctx.sampleRate);
        const data = buffer.getChannelData(0);
        for (let i = 0; i < len; i++) data[i] = (Math.random() * 2 - 1) * 0.5;
        const src = this.ctx.createBufferSource();
        src.buffer = buffer;
        const gain = this.ctx.createGain();
        gain.gain.setValueAtTime(vol, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + duration);
        const filter = this.ctx.createBiquadFilter();
        filter.type = 'bandpass';
        filter.frequency.value = 800;
        filter.Q.value = 1;
        src.connect(filter);
        filter.connect(gain);
        gain.connect(this.sfxGain);
        src.start(t);
    },

    playHit() { this.playTone(180, 0.12, 0.07, 'square'); this.playTone(90, 0.08, 0.04, 'sawtooth'); },
    playSwing() { this.playNoise(0.04, 0.08); },
    playJump() { this.playTone(320, 0.06, 0.08, 'sine'); },
    playDodge() { this.playNoise(0.05, 0.12); this.playTone(250, 0.04, 0.08, 'sine'); },
    playHurt() { this.playTone(140, 0.12, 0.2, 'sawtooth'); },
    playParry() { this.playTone(800, 0.1, 0.1, 'square'); this.playTone(1200, 0.06, 0.05, 'sine'); },
    playStealthKill() { this.playTone(400, 0.08, 0.12, 'sine'); this.playTone(600, 0.05, 0.08, 'sine'); },
    playWeb() { this.playTone(500, 0.06, 0.2, 'sine'); this.playTone(700, 0.04, 0.15, 'sine'); },
    playSpiderSense() { this.playTone(330, 0.05, 0.5, 'sine'); this.playTone(440, 0.03, 0.4, 'sine'); this.playTone(550, 0.02, 0.3, 'sine'); },
    playDecoy() { this.playTone(350, 0.05, 0.2, 'triangle'); },
    playVoiceLure() { this.playTone(392, 0.06, 0.3, 'sine'); setTimeout(() => this.playTone(440, 0.04, 0.2, 'sine'), 80); },
    playAnansiAppear() { [330,392,440,523,587,659,784].forEach((n,i) => setTimeout(() => this.playTone(n, 0.05, 0.4, 'sine'), i * 120)); },
    playPuzzle() { this.playTone(523, 0.07, 0.3, 'sine'); },
    playPuzzleSolve() { [523,587,659,784].forEach((n,i) => setTimeout(() => this.playTone(n, 0.08, 0.5, 'sine'), i * 180)); },
    playPuzzleFail() { this.playTone(200, 0.08, 0.3, 'sawtooth'); this.playTone(150, 0.06, 0.4, 'sawtooth'); }
};

// ============ UTILITIES ============
function lerp(a, b, t) { return a + (b - a) * t; }
function clamp(v, min, max) { return Math.max(min, Math.min(max, v)); }
function lerpColor(c1, c2, t) {
    const p = (c, i) => parseInt(c.slice(1 + i * 2, 3 + i * 2), 16);
    const r = Math.round(lerp(p(c1, 0), p(c2, 0), t));
    const g = Math.round(lerp(p(c1, 1), p(c2, 1), t));
    const b = Math.round(lerp(p(c1, 2), p(c2, 2), t));
    return `#${r.toString(16).padStart(2,'0')}${g.toString(16).padStart(2,'0')}${b.toString(16).padStart(2,'0')}`;
}
function roundRect(ctx, x, y, w, h, r) {
    ctx.beginPath();
    ctx.moveTo(x + r, y);
    ctx.lineTo(x + w - r, y);
    ctx.quadraticCurveTo(x + w, y, x + w, y + r);
    ctx.lineTo(x + w, y + h - r);
    ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
    ctx.lineTo(x + r, y + h);
    ctx.quadraticCurveTo(x, y + h, x, y + h - r);
    ctx.lineTo(x, y + r);
    ctx.quadraticCurveTo(x, y, x + r, y);
    ctx.closePath();
}

// Climb surface check
World.getClimbSurface = function(x, y) {
    for (const cs of this.climbSurfaces) {
        if (Math.abs(x - cs.x) < 15 && y >= cs.y && y <= cs.y + cs.h) return cs;
    }
    return null;
};

World.getLatchPoint = function(x, y, range) {
    let closest = null, dist = range;
    for (const lp of this.latchPoints) {
        const d = Math.hypot(lp.x - x, lp.y - y);
        if (d < dist) { dist = d; closest = lp; }
    }
    return closest;
};

World.getHideSpot = function(x, y) {
    for (const hs of this.hideSpots) {
        if (x >= hs.x && x <= hs.x + hs.w && y >= hs.y && y <= hs.y + hs.h) return hs;
    }
    return null;
};
