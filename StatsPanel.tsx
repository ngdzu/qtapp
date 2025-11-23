import React, { useEffect, useState } from 'react';
import { LineChart, Line, ResponsiveContainer, YAxis } from 'recharts';
import { Cpu, Database, Wifi, Zap } from 'lucide-react';
import { SystemStats } from '../types';

interface StatsPanelProps {
  stats: SystemStats;
}

const StatsPanel: React.FC<StatsPanelProps> = ({ stats }) => {
  const [data, setData] = useState<{ cpu: number; memory: number }[]>([]);

  useEffect(() => {
    setData(prev => {
      const newData = [...prev, { cpu: stats.cpu, memory: stats.memory }];
      if (newData.length > 20) newData.shift();
      return newData;
    });
  }, [stats]);

  return (
    <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
      <div className="bg-zinc-900 border border-zinc-800 p-4 rounded-lg shadow-lg relative overflow-hidden group">
        <div className="absolute -right-6 -top-6 bg-emerald-500/10 w-24 h-24 rounded-full blur-xl group-hover:bg-emerald-500/20 transition-all"></div>
        <div className="flex items-center justify-between mb-2">
          <span className="text-zinc-400 text-sm font-medium flex items-center gap-2">
            <Cpu size={16} /> Remote CPU
          </span>
          <span className="text-emerald-400 font-bold">{stats.cpu}%</span>
        </div>
        <div className="h-12 w-full">
           <ResponsiveContainer width="100%" height="100%">
             <LineChart data={data}>
               <YAxis domain={[0, 100]} hide />
               <Line type="monotone" dataKey="cpu" stroke="#10b981" strokeWidth={2} dot={false} />
             </LineChart>
           </ResponsiveContainer>
        </div>
      </div>

      <div className="bg-zinc-900 border border-zinc-800 p-4 rounded-lg shadow-lg relative overflow-hidden group">
        <div className="absolute -right-6 -top-6 bg-blue-500/10 w-24 h-24 rounded-full blur-xl group-hover:bg-blue-500/20 transition-all"></div>
        <div className="flex items-center justify-between mb-2">
          <span className="text-zinc-400 text-sm font-medium flex items-center gap-2">
            <Database size={16} /> RAM Usage
          </span>
          <span className="text-blue-400 font-bold">{stats.memory}%</span>
        </div>
         <div className="h-12 w-full">
           <ResponsiveContainer width="100%" height="100%">
             <LineChart data={data}>
               <YAxis domain={[0, 100]} hide />
               <Line type="monotone" dataKey="memory" stroke="#3b82f6" strokeWidth={2} dot={false} />
             </LineChart>
           </ResponsiveContainer>
        </div>
      </div>

      <div className="bg-zinc-900 border border-zinc-800 p-4 rounded-lg shadow-lg relative overflow-hidden group">
         <div className="absolute -right-6 -top-6 bg-purple-500/10 w-24 h-24 rounded-full blur-xl group-hover:bg-purple-500/20 transition-all"></div>
        <div className="flex items-center justify-between mb-2">
          <span className="text-zinc-400 text-sm font-medium flex items-center gap-2">
            <Zap size={16} /> GPU Load
          </span>
          <span className="text-purple-400 font-bold">{stats.gpu}%</span>
        </div>
        <div className="w-full bg-zinc-800 h-2 rounded-full mt-4 overflow-hidden">
          <div 
            className="bg-purple-500 h-full transition-all duration-500 ease-out"
            style={{ width: `${stats.gpu}%` }}
          ></div>
        </div>
      </div>

      <div className="bg-zinc-900 border border-zinc-800 p-4 rounded-lg shadow-lg relative overflow-hidden group">
        <div className="absolute -right-6 -top-6 bg-amber-500/10 w-24 h-24 rounded-full blur-xl group-hover:bg-amber-500/20 transition-all"></div>
        <div className="flex items-center justify-between mb-2">
          <span className="text-zinc-400 text-sm font-medium flex items-center gap-2">
            <Wifi size={16} /> Latency
          </span>
          <span className="text-amber-400 font-bold">24ms</span>
        </div>
        <div className="text-xs text-zinc-500 mt-3">Connection: Stable (Tailscale)</div>
      </div>
    </div>
  );
};

export default StatsPanel;